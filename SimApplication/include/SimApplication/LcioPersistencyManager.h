#ifndef SIM_APPLICATION_LCIOPERSISTENCYMANAGER_H_
#define SIM_APPLICATION_LCIOPERSISTENCYMANAGER_H_

/*
 * LCDD
 */
#include "lcdd/core/LCDDProcessor.hh"
#include "lcdd/hits/CalorimeterHit.hh"
#include "lcdd/hits/TrackerHit.hh"

/*
 * Geant4
 */
#include "G4RunManager.hh"
#include "G4PersistencyManager.hh"
#include "G4PersistencyCenter.hh"
#include "G4Run.hh"
#include "G4HCofThisEvent.hh"
#include "G4SDManager.hh"
#include "G4SystemOfUnits.hh"

/*
 * LCIO
 */
#include "Exceptions.h"
#include "IO/LCWriter.h"
#include "IMPL/LCEventImpl.h"
#include "IMPL/LCCollectionVec.h"
#include "IMPL/LCFlagImpl.h"
#include "IMPL/LCRunHeaderImpl.h"
#include "IMPL/MCParticleImpl.h"
#include "IMPL/SimCalorimeterHitImpl.h"
#include "IMPL/SimTrackerHitImpl.h"
#include "EVENT/LCIO.h"
#include "IOIMPL/LCFactory.h"
#include "UTIL/LCTOOLS.h"

/*
 * HPS
 */
#include "LcioMergeTool.h"
#include "LcioPersistencyMessenger.h"
//#include "MCParticleBuilder.h"

/*
 * C++
 */
#include <map>

/*
 * LCIO
 */
using IMPL::LCCollectionVec;
using EVENT::LCIO;

namespace ldmx {

/**
 * @class LcioPersistencyManager
 * @brief Manages persistence of Geant4 objects to an LCIO output file
 * @see http://lcio.desy.de/
 */
class LcioPersistencyManager : public G4PersistencyManager {

    public:

        /** File write mode. */
        enum WriteMode {
            /** Make a new file and throw an error if it exists already. */
            NEW = -1,
            /** Make a new file and overwrite an existing one if it exists. */
            RECREATE = LCIO::WRITE_NEW,
            /** Append to an existing file. */
            APPEND = LCIO::WRITE_APPEND
        };

        /**
         * Class constructor, which will register this persistency manager as the global default within Geant4.
         */
        LcioPersistencyManager();

        virtual ~LcioPersistencyManager();

        /**
         * Get the global instance of the persistency manager.
         */
        static LcioPersistencyManager* getInstance();

        /**
         * Store a Geant4 event to an LCIO output event.
         *
         * @note Events marked as aborted are skipped and not stored.
         */
        G4bool Store(const G4Event* anEvent);

        /**
         * End of run hook which is used to close the current LCIO writer.
         */
        G4bool Store(const G4Run* aRun);

        G4bool Store(const G4VPhysicalVolume*);

        /**
         * Initialize an object of this class at the beginning of the run.
         * Opens an LCIO file for writing using the current file name and write mode.
         */
        void Initialize();

        /**
         * Set the name of the output file.
         */
        void setOutputFile(std::string outputFile);

        /**
         * Set the WriteMode of the LCIO writer.
         */
        void setWriteMode(WriteMode writeMode);

        /**
         * Convert a string to a WriteMode enum value.
         */
        const std::string& modeToString(WriteMode writeMode);

        /**
         * Add an LCIO file to merge into the output event during processing.
         */
        void addMerge(LcioMergeTool* merge);

        /**
         * Get the named merge configuration.
         */
        LcioMergeTool* getMerge(std::string name);

        /**
         * Turn on dump of event summary during processing.
         */
        void setDumpEventSummary(bool dumpEventSummary);

        /**
         * Turn on detailed dump during processing.
         */
        void setDumpEventDetailed(bool dumpEventDetailed);

        /**
         * Dump detailed collection data for a single file.
         */
        static void dumpFile(std::string fileName,
                int nevents = -1,
                int nskip = 0);

    private:

        /**
         * Write hits collections from the Geant4 event to an LCIO event.
         */
        void writeHitsCollections(const G4Event* g4Event, IMPL::LCEventImpl* lcioEvent);

        /**
         * Write a TrackerHitsCollection (LCDD) to LCIO.
         */
        IMPL::LCCollectionVec* writeTrackerHitsCollection(G4VHitsCollection* hc);

        /**
         * Write a CalorimeterHitsCollection (LCDD) to LCIO.
         */
        IMPL::LCCollectionVec* writeCalorimeterHitsCollection(G4VHitsCollection* hc);

        /**
         * Dump an event summary and/or detailed information depending on the
         * current flag settings.
         */
        void dumpEvent(EVENT::LCEvent* event);

    private:

        /** Name of the output file. */
        std::string outputFile_{"hps_sim_events.slcio"};

        /** The current LCIO data writer. */
        IO::LCWriter* writer_;

        /** Builds MCParticle collection for the persistency manager. */
        //MCParticleBuilder* builder_;

        /** Messenger for macro command processing. */
        LcioPersistencyMessenger* messenger_;

        /** LCIO write mode. */
        WriteMode writeMode_{NEW};

        /** LCIO files to merge into every Geant4 event (optional). */
        std::map<std::string, LcioMergeTool*> merge_;

        /** Flag to dump collection summary info after writing an event. */
        bool dumpEventSummary_{false};

        /** Flag to dump detailed collection info after writing an event. */
        bool dumpEventDetailed_{false};

};


}

#endif
