#ifndef SIM_APPLICATION_LCIOMERGETOOL_H_
#define SIM_APPLICATION_LCIOMERGETOOL_H_

/*
 * Geant4
 */
#include "G4SystemOfUnits.hh"

/*
 * LCIO
 */
#include "EVENT/LCCollection.h"
#include "IMPL/LCEventImpl.h"
#include "IMPL/LCCollectionVec.h"
#include "IMPL/SimCalorimeterHitImpl.h"
#include "IO/LCReader.h"
#include "IOIMPL/LCFactory.h"

/*
 * HPS
 */
#include "LcioMergeMessenger.h"

namespace ldmx {

/**
 * @class LcioMergeTool
 * @brief Tool for merging LCIO events
 * @note This class is meant to configure a single LCIO event stream.
 * If there are multiple LCIO event streams being merged, then an instance
 * of this class should be created for each one.
 */
class LcioMergeTool {

    public:

        /**
         * Pair of 32-bit IDs used to uniquely identify hits.
         */
        typedef std::pair<int, int> CellID;

        /**
         * @class MergeFilter
         * @brief Simple interface for accepting or rejecting merge events.
         */
        class MergeFilter {

            public:

                /**
                 * Return true if the filter should accept this source event.
                 */
                virtual bool accept(EVENT::LCEvent*) {
                    return true;
                }

                /**
                 * Return true if the filter should skip this target event,
                 * which means that no events will be merged into it from this
                 * source.
                 */
                virtual bool skip(EVENT::LCEvent*) {
                    return false;
                }
        };

        /**
         * @class EcalEnergyFilter
         * @brief Rejects source events if they do not have sufficient energy deposition in the ECal.
         */
        class EcalEnergyFilter : public MergeFilter {

            public:

                bool accept(EVENT::LCEvent* event) {
                    auto hits = event->getCollection(collName_);
                    float e=0;
                    for (int iElem = 0; iElem < hits->getNumberOfElements(); iElem++) {
                        EVENT::SimCalorimeterHit* hit =
                                static_cast<EVENT::SimCalorimeterHit*>(hits->getElementAt(iElem));
                        e += hit->getEnergy() * GeV;
                    }
                    return e >= energyCut_;
                }

                void setEnergyCut(float energyCut) {
                    energyCut_ = energyCut;
                }

            private:

                float energyCut_{50 * MeV};
                std::string collName_{"EcalHits"};
        };
        
        /**
         * @class EventModoulusFilter 
         * @brief Skips target events if a modulus does not divide evenly into their event numbers.
         */
        class EventModulusFilter : public MergeFilter {

            public:

                virtual bool skip(EVENT::LCEvent* event) {
                    return event->getEventNumber() % modulus_ != 0; 
                }

                void setModulus(int modulus) {
                    modulus_ = modulus;
                }

            private:

                int modulus_{1};
        };

    public:

        LcioMergeTool(std::string name) : name_(name) {
            messenger_ = new LcioMergeMessenger(this);
        }

        virtual ~LcioMergeTool() {
            if (reader_) {
                try {
                    reader_->close();
                } catch (std::exception& e) {
                    std::cerr << e.what() << std::endl;
                }
                delete reader_;
            }

            delete messenger_;
        }

        const std::string& getName() {
            return name_;
        }

        /**
         * Set the verbose level.
         *
         * @note The LcioPersistencyManager will copy its verbose level to the registered
         * LcioMergeTool objects during beginning-of-run initialization.
         */
        void setVerbose(int verbose) {
            verbose_ = verbose;
        }

        /**
         * Set whether SimCalorimeterHit objects with the same cell IDs should be combined
         * into a single output hit.
         */
        void setCombineCalHits(bool combineCalHits) {
            combineCalHits_ = combineCalHits;
        }

        /**
         * Merge source event into target event.
         *
         * If writeColls is not empty then only collection names that it
         * contains will be written out to the target event.
         */
        void mergeEvent(EVENT::LCEvent* src, IMPL::LCEventImpl* target, const std::vector<std::string>& writeColls) {

            // Names of collections in source event.
            const std::vector<std::string>* srcCollNames = src->getCollectionNames();

            // Names of collections in target event.
            const std::vector<std::string>* targetCollNames = src->getCollectionNames();

            // Process source collection names.
            for (auto collName : *srcCollNames) {

                // Check if this collection should be written.
                if (writeColls.size() == 0 || 
                        std::find(writeColls.begin(), writeColls.end(), collName) != writeColls.end()) {

                    // Get source collection and take ownership so it is not automatically deleted by the reader.
                    auto srcColl = src->getCollection(collName);
                    src->takeCollection(collName);

                    // Get target collection from output event if it exists, or create new one if not.
                    IMPL::LCCollectionVec* targetColl = nullptr;
                    bool createdNewCollection = false;
                    if (std::find(targetCollNames->begin(), targetCollNames->end(), collName) != targetCollNames->end()) {
                        targetColl = (IMPL::LCCollectionVec*) target->getCollection(collName);
                    } else {
                        targetColl = new IMPL::LCCollectionVec(srcColl->getTypeName());
                        target->addCollection(targetColl, collName);
                        createdNewCollection = true;
                    }

                    // Add all elements from source to target collection.
                    addElements(srcColl, targetColl);

                    // Combine SimCalorimeterHit objects in place.
                    if (srcColl->getTypeName() == EVENT::LCIO::SIMCALORIMETERHIT
                            && combineCalHits_ && !createdNewCollection
                            && srcColl->getNumberOfElements() != targetColl->getNumberOfElements()) {
                        if (verbose_ > 1) {
                            std::cout << "LcioMergeTool: Combining " << targetColl->getNumberOfElements() << " hits in '"
                                    << collName << "'" << std::endl;
                        }
                        combine(targetColl);
                        if (verbose_ > 1) {
                            std::cout << "LcioMergeTool: Created " << targetColl->getNumberOfElements() << " combined cal hits" << std::endl;
                        }
                    }

                    // Clear and delete the source collection which we took from the event.
                    clear(srcColl);
                    delete srcColl;
                }
            }
        }

        /**
         * Clear all data members of an LCCollection.
         */
        static void clear(EVENT::LCCollection* coll) {
            for (int iElem = coll->getNumberOfElements() - 1; iElem >= 0; iElem--) {
                coll->removeElementAt(iElem);
            }
        }

        /**
         * Merge source event into target event, including all collections.
         */
        void mergeEvent(EVENT::LCEvent* src, IMPL::LCEventImpl* target) {
            static std::vector<std::string> allCollections;
            mergeEvent(src, target, allCollections);
        }

        /**
         * Merge one event from the reader into the target output event,
         * applying any event filters to read events until one is found
         * that passes.
         */
        // FIXME: Check for event underflow when reading events!
        void mergeEvents(IMPL::LCEventImpl* target) {

            // check if merge filter wants to skip this output event
            if (filters_.size()) {
                if (skip(target, filters_)) {
                    return;
                }
            }

            // read next src event
            auto event = reader_->readNextEvent(EVENT::LCIO::UPDATE);

            // if necessary read events until one passes the filters
            if (filters_.size()) {
                bool acceptEvent = accept(event, filters_);
                while (!acceptEvent) {
                    if (verbose_ > 2) {
                        std::cout << "LcioMergeTool: Event " << event->getEventNumber()
                                << " rejected by filters of '" << getName() << "'" << std::endl;
                    }
                    event = reader_->readNextEvent(EVENT::LCIO::UPDATE);
                    acceptEvent = accept(event, filters_);
                }
                if (verbose_ > 2) {
                    std::cout << "LcioMergeTool: Event " << event->getEventNumber()
                            << " accepted by filters of '" << getName() << "'" << std::endl;
                }
            }

            // finally merge source event after filtering to target
            mergeEvent(event, target);
        }

        /**
         * Add a file from which to merge.
         */
        void addFile(std::string file) {
            files_.push_back(file);
        }

        /**
         * Add an event filter.
         */
        void addFilter(MergeFilter* filter) {
            filters_.push_back(filter);
        }

        /**
         * Open the list of files using the reader.
         */
        void initialize() {
            if (reader_) {
                reader_->close();
                delete reader_;
            }
            reader_ = IOIMPL::LCFactory::getInstance()->createLCReader();
            reader_->open(files_);
        }

    private:

        /**
         * Apply event filters to an input LCIO event, rejecting events that are not accepted
         * by all filters.
         */
        bool accept(EVENT::LCEvent* event, const std::vector<MergeFilter*>& filters) {
            for (MergeFilter* filter : filters) {
                if (!filter->accept(event)) {
                    return false;
                }
            }
            return true;
        }

        /**
         * Returns true if event filters request to skip this output event.
         */
        bool skip(EVENT::LCEvent* event, const std::vector<MergeFilter*>& filters) {
            for (MergeFilter* filter : filters) {
                if (filter->skip(event)) {
                    return true;
                }
            }
            return false;
        }

        /**
         * Add all elements from one collection to another.
         */
        void addElements(EVENT::LCCollection* src, IMPL::LCCollectionVec* target) {
            for (int iElem = 0; iElem < src->getNumberOfElements(); iElem++) {
                target->addElement(src->getElementAt(iElem));
            }
        }

        /**
         * Combine all SimCalorimeterHit objects with the same cell IDs into a single set of hits.
         */
        void combine(IMPL::LCCollectionVec* hits) {

            // create a list with combined hits
            std::set<CellID> processedHits;
            auto combinedColl = std::vector<EVENT::SimCalorimeterHit*>();
            for (int iElem = 0; iElem < hits->getNumberOfElements(); iElem++) {
                EVENT::SimCalorimeterHit* hit =
                        static_cast<EVENT::SimCalorimeterHit*>(hits->getElementAt(iElem));
                CellID id = CellID(hit->getCellID0(), hit->getCellID1());
                if (processedHits.find(id) == processedHits.end()) {
                    auto foundHits = find(hit, hits);
                    IMPL::SimCalorimeterHitImpl* combinedHit = createSingleHit(foundHits);
                    combinedColl.push_back(combinedHit);
                    processedHits.insert(id);
                }
            }

            // clear the old collection
            clear(hits);

            // add combined hits back to the collection
            for (auto hit : combinedColl) {
                hits->addElement(hit);
            }
        }

        /**
         * Create a single hit from a list of input hits that have the same cell IDs.
         */
        IMPL::SimCalorimeterHitImpl* createSingleHit(const std::vector<EVENT::SimCalorimeterHit*>& hits) {
            if (!hits.size()) {
                G4Exception("LcioMergeTool::combine", "", FatalException, "Hit vector is empty.");
            }
            auto combinedHit = new IMPL::SimCalorimeterHitImpl;
            auto firstHit = hits.at(0);
            combinedHit->setCellID0(firstHit->getCellID0());
            combinedHit->setCellID1(firstHit->getCellID1());
            combinedHit->setPosition(firstHit->getPosition());
            for (auto hit : hits) {
                int nContrib = hit->getNMCContributions();
                for (int iContrib = 0; iContrib < nContrib; iContrib++) {
                    combinedHit->addMCParticleContribution(
                            hit->getParticleCont(iContrib),
                            hit->getEnergyCont(iContrib),
                            hit->getTimeCont(iContrib),
                            hit->getPDGCont(iContrib));
                }
            }
            return combinedHit;
        }

        /**
         * Find hits matching a cell ID of a hit.
         */
        std::vector<EVENT::SimCalorimeterHit*> find(EVENT::SimCalorimeterHit* searchHit, EVENT::LCCollection* coll) {
            std::vector<EVENT::SimCalorimeterHit*> foundHits;
            for (int iElem = 0; iElem < coll->getNumberOfElements(); iElem++) {
                EVENT::SimCalorimeterHit* hit = static_cast<EVENT::SimCalorimeterHit*>(coll->getElementAt(iElem));
                if (searchHit->getCellID0() == hit->getCellID0() && searchHit->getCellID1() == hit->getCellID1()) {
                    foundHits.push_back(hit);
                }
            }
            return foundHits;
        }

    private:

        G4UImessenger* messenger_;
        std::string name_;
        IO::LCReader* reader_{nullptr};
        std::vector<std::string> files_;
        std::vector<MergeFilter*> filters_;
        bool combineCalHits_{true};
        int verbose_{1};
};

}

#endif
