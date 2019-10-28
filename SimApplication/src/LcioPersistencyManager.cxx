#include "SimApplication/LcioPersistencyManager.h"

#include "SimApplication/UserTrackingAction.h" 

namespace ldmx {

LcioPersistencyManager::LcioPersistencyManager() :
        G4PersistencyManager(G4PersistencyCenter::GetPersistencyCenter(), "LcioPersistencyManager") {
    G4PersistencyCenter::GetPersistencyCenter()->RegisterPersistencyManager(this);
    G4PersistencyCenter::GetPersistencyCenter()->SetPersistencyManager(this, "LcioPersistencyManager");
    writer_ = nullptr;
    builder_ = new LcioSimParticleBuilder(UserTrackingAction::getUserTrackingAction()->getTrackMap()); // FIXME: Probably shouldn't set this here!
    messenger_ = new LcioPersistencyMessenger(this);
}

LcioPersistencyManager::~LcioPersistencyManager() {

    if (writer_) {
        delete writer_;
    }

    delete builder_;
    delete messenger_;

    for (auto entry : merge_) {
        delete entry.second;
    }
    merge_.clear();
}

/**
 * Get the global instance of the persistency manager.
 */
LcioPersistencyManager* LcioPersistencyManager::getInstance() {
    return (LcioPersistencyManager*) G4PersistencyCenter::GetPersistencyCenter()->CurrentPersistencyManager();
}

/**
 * Store a Geant4 event to an LCIO output event.
 *
 * @note Events marked as aborted are skipped and not stored.
 */
G4bool LcioPersistencyManager::Store(const G4Event* anEvent) {
    if (!anEvent->IsAborted()) {

        if (m_verbose > 1) {
            std::cout << "LcioPersistencyManager: Storing event " << anEvent->GetEventID() << std::endl;
        }

        // Create new LCIO event.
        IMPL::LCEventImpl* lcioEvent = new IMPL::LCEventImpl();
        lcioEvent->setEventNumber(anEvent->GetEventID());
        lcioEvent->setRunNumber(G4RunManager::GetRunManager()->GetCurrentRun()->GetRunID());
        lcioEvent->setDetectorName(LCDDProcessor::instance()->getDetectorName());
        if (anEvent->GetPrimaryVertex()) {
            lcioEvent->setWeight(anEvent->GetPrimaryVertex()->GetWeight());
            if (m_verbose > 1) {
                std::cout << "LcioPersistencyManager: Set LCIO event weight to " << lcioEvent->getWeight() << std::endl;
            }
        }

        // Write MCParticles to LCIO event (allowed to be empty).
        auto particleColl = builder_->buildMCParticleColl(anEvent);
        if (m_verbose > 1) {
            std::cout << "LcioPersistencyManager: Storing " << particleColl->size() << " MC particles in event "
                    << anEvent->GetEventID() << std::endl;
        }
        lcioEvent->addCollection(particleColl, EVENT::LCIO::MCPARTICLE);
        

        // Write hits collections to LCIO event.
        writeHitsCollections(anEvent, lcioEvent);

        // Optionally apply LCIO event merging into output event.
        if (this->merge_.size()) {
            for (auto entry : merge_) {
                if (m_verbose > 1) {
                    std::cout << "LcioPersistencyManager: Merging from '" << entry.first << "' into event "
                            << anEvent->GetEventID() << std::endl;
                }
                entry.second->mergeEvents(lcioEvent);
            }
        }

        // Write event and flush writer.
        writer_->writeEvent(static_cast<EVENT::LCEvent*>(lcioEvent));
        writer_->flush();

        // Print final number of objects in collections, including those added by merging LCIO files.
        if (m_verbose > 1) {
            for (auto collName : *lcioEvent->getCollectionNames()) {
                try {
                    EVENT::LCCollection* coll = lcioEvent->getCollection(collName);
                    std::cout << "LcioPersistencyManager: Stored " << coll->getNumberOfElements() << " objects in '"
                            << collName << "'" << std::endl;
                } catch (EVENT::DataNotAvailableException& e) {
                    std::cerr << e.what() << std::endl;
                }
            }
        }

        // Dump event information (optional).
        dumpEvent(lcioEvent);

        // Delete the event object to avoid memory leak.
        delete lcioEvent;

        return true;

    } else {
        if (m_verbose > 1) {
            std::cout << "LcioPersistencyManager: Skipping aborted event " << anEvent->GetEventID() << std::endl;
        }
        return false;
    }
}

/**
 * End of run hook which is used to close the current LCIO writer.
 */
G4bool LcioPersistencyManager::Store(const G4Run* aRun) {
    if (m_verbose > 1) {
        std::cout << "LcioPersistencyManager: Store run " << aRun->GetRunID() << std::endl;
    }

    writer_->close();

    return true;
}

G4bool LcioPersistencyManager::Store(const G4VPhysicalVolume*) {
    return false;
}

/**
 * Initialize an object of this class at the beginning of the run.
 * Opens an LCIO file for writing using the current file name and write mode.
 */
void LcioPersistencyManager::Initialize() {

    if (m_verbose > 1) {
        std::cout << "LcioPersistencyManager: Initializing the persistency manager" << std::endl;
    }

    // Open output writer with configured mode.
    if (m_verbose > 1) {
        std::cout << "LcioPersistencyManager: Opening '" << outputFile_ << "' with mode " << modeToString(writeMode_)
                << std::endl;
    }
    writer_ = IOIMPL::LCFactory::getInstance()->createLCWriter();
    try {
        if (writeMode_ == NEW) {
            writer_->open(outputFile_);
        } else {
            writer_->open(outputFile_, writeMode_);
        }
    } catch (IO::IOException& e) {
        G4Exception("LcioPersistencyManager::Initialize()", "FileExists", RunMustBeAborted, e.what());
    }

    // Create run header and write to beginning of output file.
    auto runHeader = new IMPL::LCRunHeaderImpl();
    runHeader->setDetectorName(LCDDProcessor::instance()->getDetectorName());
    runHeader->setRunNumber(G4RunManager::GetRunManager()->GetCurrentRun()->GetRunID());
    runHeader->setDescription("HPS MC events");
    writer_->writeRunHeader(static_cast<EVENT::LCRunHeader*>(runHeader));

    // Initialize file merge tools.
    for (auto entry : merge_) {
        if (m_verbose > 1) {
            std::cout << "LcioPersistencyManager: Initializing merge tool '" << entry.second->getName() << "'"
                    << std::endl;
        }
        entry.second->setVerbose(m_verbose);
        entry.second->initialize();
    }
}

/**
 * Set the name of the output file.
 */
void LcioPersistencyManager::setOutputFile(std::string outputFile) {
    outputFile_ = outputFile;
}

/**
 * Set the WriteMode of the LCIO writer.
 */
void LcioPersistencyManager::setWriteMode(WriteMode writeMode) {
    writeMode_ = writeMode;
}

/**
 * Convert a string to a WriteMode enum value.
 */
const std::string& LcioPersistencyManager::modeToString(WriteMode writeMode) {
    static std::vector<std::string> writeModes { "NEW", "RECREATE", "APPEND" };
    if (writeMode == NEW) {
        return writeModes[0];
    } else if (writeMode == RECREATE) {
        return writeModes[1];
    } else if (writeMode == APPEND) {
        return writeModes[2];
    } else {
        G4Exception("LcioPersistencyManager::modeToString", "", FatalException,
                G4String("Unknown write mode: " + writeMode));
    }
    // This will never happen.
    static std::string empty("");
    return empty;
}

/**
 * Add an LCIO file to merge into the output event during processing.
 */
void LcioPersistencyManager::addMerge(LcioMergeTool* merge) {
    merge_[merge->getName()] = merge;
}

/**
 * Get the named merge configuration.
 */
LcioMergeTool* LcioPersistencyManager::getMerge(std::string name) {
    if (merge_.find(name) != merge_.end()) {
        return merge_[name];
    } else {
        return nullptr;
    }
}

/**
 * Turn on dump of event summary during processing.
 */
void LcioPersistencyManager::setDumpEventSummary(bool dumpEventSummary) {
    dumpEventSummary_ = dumpEventSummary;
}

/**
 * Turn on detailed dump during processing.
 */
void LcioPersistencyManager::setDumpEventDetailed(bool dumpEventDetailed) {
    dumpEventDetailed_ = dumpEventDetailed;
}

/**
 * Dump detailed collection data for a single file.
 */
void LcioPersistencyManager::dumpFile(std::string fileName, int nevents, int nskip) {
    auto reader = IOIMPL::LCFactory::getInstance()->createLCReader();
    reader->open(fileName);
    if (nskip > 0) {
        reader->skipNEvents(nskip);
    }
    int nread = 0;
    while (nread < nevents || nevents == -1) {
        auto event = reader->readNextEvent();
        if (!event) {
            break;
        }
        UTIL::LCTOOLS::dumpEventDetailed(event);
        ++nread;
    }
    try {
        reader->close();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    delete reader;
}

/**
 * Write hits collections from the Geant4 event to an LCIO event.
 */
void LcioPersistencyManager::writeHitsCollections(const G4Event* g4Event, IMPL::LCEventImpl* lcioEvent) {
    G4HCofThisEvent* hce = g4Event->GetHCofThisEvent();
    if (hce) {
        int nColl = hce->GetNumberOfCollections();

        for (int iColl = 0; iColl < nColl; iColl++) {

            G4VHitsCollection* hc = hce->GetHC(iColl);
            std::string collName = hc->GetName();
            LCCollectionVec* collVec = nullptr;

            if (dynamic_cast<TrackerHitsCollection*>(hc)) {
                collVec = writeTrackerHitsCollection(hc);
            } else if (dynamic_cast<CalorimeterHitsCollection*>(hc)) {
                collVec = writeCalorimeterHitsCollection(hc);
            } else {
                std::cerr << "Hits collection '" << collName << "' has unknown type." << std::endl;
                G4Exception("LcioPersistencyManager::writeHitsCollections", "", FatalException, "Unknown hit type.");
            }

            if (collVec) {
                lcioEvent->addCollection(collVec, collName);
                if (m_verbose > 1) {
                    std::cout << "LcioPersistencyManager: Stored " << collVec->size() << " hits in '" << collName << "'"
                            << std::endl;
                }
            }
        }
    }
}

/**
 * Write a TrackerHitsCollection (LCDD) to LCIO.
 */
IMPL::LCCollectionVec* LcioPersistencyManager::writeTrackerHitsCollection(G4VHitsCollection* hc) {
    auto trackerHits = dynamic_cast<TrackerHitsCollection*>(hc);
    auto collVec = new LCCollectionVec(LCIO::SIMTRACKERHIT);
    IMPL::LCFlagImpl collFlag;
    collFlag.setBit(EVENT::LCIO::THBIT_MOMENTUM);
    collVec->setFlag(collFlag.getFlag());

    int nhits = trackerHits->GetSize();
    //if (m_verbose > 2) {
        std::cout << "LcioPersistencyManager: Converting " << nhits << " tracker hits to LCIO" << std::endl;
    //}
    for (int i = 0; i < nhits; i++) {

        auto trackerHit = static_cast<TrackerHit*>(trackerHits->GetHit(i));
        auto simTrackerHit = new IMPL::SimTrackerHitImpl();

        // position in mm
        const G4ThreeVector posVec = trackerHit->getPosition();
        double pos[3] = { posVec.x(), posVec.y(), posVec.z() };
        simTrackerHit->setPosition(pos);
        std::cout << "x: " << posVec.x() << std::endl;

        // momentum in GeV
        const G4ThreeVector& momentum = trackerHit->getMomentum();
        simTrackerHit->setMomentum(momentum.x() / GeV, momentum.y() / GeV, momentum.z() / GeV);

        // pathLength = distance between exit and entry points in mm
        simTrackerHit->setPathLength(trackerHit->getLength());

        // dEdx in GeV (LCIO units)
        float edep = trackerHit->getEdep();
        simTrackerHit->setEDep(edep / GeV);

        // time in NS
        float tEdep = trackerHit->getTdep();
        simTrackerHit->setTime(tEdep);

        // Cell ID.
        simTrackerHit->setCellID0(trackerHit->getId());
        collVec->push_back(simTrackerHit);

        // get the MCParticle for the hit
        //if (m_verbose > 3) {
        /*    std::cout << "LcioPersistencyManager: Looking for track ID " << trackerHit->getTrackID() << std::endl;
        //}
        IMPL::MCParticleImpl* mcp = builder_->findMCParticle(trackerHit->getTrackID());
        if (!mcp) {
            std::cerr << "LcioPersistencyManager: No MCParticle found for trackID " << trackerHit->getTrackID()
                    << " from sim tracker hit" << std::endl;
            G4Exception("LcioPersistencyManager::writeHitsCollections", "", FatalException,
                    "MCParticle for track ID is missing.");
        } else {
            simTrackerHit->setMCParticle(mcp);
        }*/
    }
    return collVec;
}

/**
 * Write a CalorimeterHitsCollection (LCDD) to LCIO.
 */
IMPL::LCCollectionVec* LcioPersistencyManager::writeCalorimeterHitsCollection(G4VHitsCollection* hc) {

    auto calHits = dynamic_cast<CalorimeterHitsCollection*>(hc);
    auto collVec = new LCCollectionVec(LCIO::SIMCALORIMETERHIT);
    IMPL::LCFlagImpl collFlag;
    collFlag.setBit(EVENT::LCIO::CHBIT_LONG);
    collFlag.setBit(EVENT::LCIO::CHBIT_PDG);
    collVec->setFlag(collFlag.getFlag());

    int nhits = calHits->GetSize();
    if (m_verbose > 2) {
        std::cout << "LcioPersistencyManager: Converting " << nhits << " calorimeter hits to LCIO" << std::endl;
    }
    for (int i = 0; i < nhits; i++) {

        auto calHit = static_cast<CalorimeterHit*>(calHits->GetHit(i));
        auto simCalHit = new IMPL::SimCalorimeterHitImpl();

        // set cellid from cal hit's id64
        const Id64bit& id64 = calHit->getId64bit();
        simCalHit->setCellID0(id64.getId0());
        simCalHit->setCellID1(id64.getId1());

        // position
        const G4ThreeVector hitPos = calHit->getPosition();
        float pos[3] = { (float) hitPos.x(), (float) hitPos.y(), (float) hitPos.z() };
        simCalHit->setPosition(pos);

       // add to output collection
        collVec->push_back(simCalHit);

        auto contribs = calHit->getHitContributions();
        /*for (auto contrib : contribs) {
            auto edep = contrib.getEdep()/GeV;
            auto hitTime = contrib.getGlobalTime();
            auto pdg = contrib.getPDGID();
            auto contribPos = contrib.getPosition();
            auto trackID = contrib.getTrackID();

            if (trackID <= 0) {
                std::cerr << "LcioPersistencyManager: Bad track ID " << trackID << " for calorimeter hit contrib"
                        << std::endl;
                G4Exception("LcioPersistencyManager::writeCalorimeterHitsCollections", "", FatalException,
                        "Bad track ID in cal hit contribution.");
            }

            // Find the first parent track with a trajectory; it could actually be this track.
            G4VTrajectory* traj = builder_->getTrackMap().findTrajectory(trackID);
            if (!traj) {
                std::cerr << "LcioPersistencyManager: No trajectory found for track ID " << trackID << std::endl;
                G4Exception("LcioPersistencyManager::writeCalorimeterHitsCollections", "", FatalException,
                        "No trajectory found for track ID.");
            }

            // Lookup an MCParticle from the parent, which should exist!
            auto mcp = builder_->findMCParticle(traj->GetTrackID());
            if (!mcp) {
                std::cerr << "LcioPersistencyManager: No MCParticle found for track ID " << trackID << std::endl;
                G4Exception("LcioPersistencyManager::writeCalorimeterHitsCollection", "", FatalException,
                        "No MCParticle found for track ID.");
            }

            simCalHit->addMCParticleContribution(static_cast<EVENT::MCParticle*>(mcp), (float) edep, (float) hitTime); //  (int) pdg, (float*) contribPos);
            if (m_verbose > 3) {
                std::cout << "LcioPersistencyManager: Assigned hit contrib with " << "trackID = " << trackID << "; "
                        << "edep = " << edep << "; " << "time = " << hitTime << "; " << "pdg = " << pdg << "; "
                        << "pos = ( " << contribPos[0] << ", " << contribPos[1] << ", " << contribPos[2] << " ) "
                        << std::endl;
            }
        }*/
    }
    return collVec;
}

/**
 * Dump an event summary and/or detailed information depending on the
 * current flag settings.
 */
void LcioPersistencyManager::dumpEvent(EVENT::LCEvent* event) {
    if (dumpEventSummary_) {
        UTIL::LCTOOLS::dumpEvent(event);
    }
    if (dumpEventDetailed_) {
        UTIL::LCTOOLS::dumpEventDetailed(event);
    }
}

}

