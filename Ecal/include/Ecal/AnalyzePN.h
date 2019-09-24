/**
 * @file AnalyzePN.h
 * @brief Energy histograms to analyze how PN interactions affect showers in ECAL
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef ECAL_ANALYZEPN_H
#define ECAL_ANALYZEPN_H

//LDMX Framework
#include "Event/EventDef.h"
#include "Framework/EventProcessor.h" //Needed to declare processor
#include "Framework/ParameterSet.h" // Needed to import parameters from configuration file

//ROOT
#include "TH2.h"

namespace ldmx {
    
    /**
     * @class AnalyzePN
     * @brief Energy histograms to analyze how PN interactions affect showers in ECAL
     */
    class AnalyzePN : public ldmx::Analyzer {
        public:

            AnalyzePN(const std::string& name, ldmx::Process& process) : ldmx::Analyzer(name, process) {}

            virtual void configure(const ldmx::ParameterSet& ps);

            virtual void analyze(const ldmx::Event& event);

            virtual void onFileOpen() { }

            virtual void onFileClose() { }

            virtual void onProcessStart(); 

            virtual void onProcessEnd();

        private:

            //checks tagger sim hits for weird behavior
            //  basically tries to veto tagger veto events
            bool taggerVetoed( const TClonesArray *taggerSimHits ) const;

            //get energy of electron in last layer of tagger
            double electronTaggerEnergy( const TClonesArray *taggerSimHits ) const;

            //totals the non-noise reconstructed energy in ECAL
            double calculateReconEnergy( const TClonesArray *ecalHitColl ) const;

            //returns true if particle is considered as "going PN"
            //  checks if any children of particle has processType photonNuclear
            bool goesPN( const SimParticle *particle ) const;

            //Python Configuration Parameters
            std::string simParticlesCollName_; //name of collection for sim particles to analyze
            std::string simParticlesPassName_; //name of pass for sim particles
            std::string ecalDigiCollName_; //name of collection to calculate ecal digis
            std::string ecalDigiPassName_; //name of pass to get ecal digis collection
            std::string taggerSimHitsCollName_; //name of collection to calculate tagger sim hits
            std::string taggerSimHitsPassName_; //name of pass to get tagger sim hits collection
            double minPrimaryPhotonEnergy_; //minimum energy to allow a photon to be labled the primary photon
            double upstreamLossThresh_; //minimum fraction of primary electron's energy to allow through to target
            double lowReconEnergy_; //definitive low Recon E for determining if event is saved
            double lowPNEnergy_; //definitie low PN energy for determining if event is saved

            //Persistence Counters
            int lowReconLowPN_; //counter for low recon energy in a low PN event
            int skippedEvents_; //counter for number of events that were skipped
            int skippedBecausePrimaryLostEnergy_; //counter for events where primary lost energy

            //ROOT Histograms
            TH2F *h_ReconE_TaggerElecE; //recon energy vs energy of electron in last layer of tagger
            TH2F *h_ReconE_HardestPN_All; //recon energy vs energy of hardest photon going PN
            TH2F *h_ReconE_TotalPN_All; //recon energy vs total energy of photons going PN
            //recon energy vs energy of hardest photon going PN excluding NoPN and PrimaryPhoton events
            TH2F *h_ReconE_HardestPN_NotSpecial;
            //recon energy vs total energy of photons going PN excluding NoPN and PrimaryPhoton events
            TH2F *h_ReconE_TotalPN_NotSpecial;
            TH1F *h_ReconE_NoPN; //recon energy when there is no PN in event
            TH1F *h_ReconE_PrimPhoton; //recon energy when primary photon went PN
    };
}

#endif /* ECAL_ANALYZEPN_H */
