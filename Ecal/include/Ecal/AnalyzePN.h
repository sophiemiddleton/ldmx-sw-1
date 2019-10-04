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

            //get energy and pT of electron in last layer of tagger
            //return pass/fail depending on energy/pT cuts
            bool electronTaggerEnergy( const TClonesArray *taggerSimHits , double &electronE, double &electronPT ) const;

            //totals the non-noise reconstructed energy in ECAL
            double calculateReconEnergy( const TClonesArray *ecalHitColl ) const;

            //checks if particle is unphysical wide angle brem by checking three things:
            //  1) if it is a photon
            //  2) if it originated in the target
            //  3) if the momentum transfer is greater than the scale of the nucleus
            //  The minimum momentum transfer for a photon of energy k and polar angle theta is
            //      q_min = E_0 k theta^2 / (4 (E_0-k))
            //      where E_0 is energy of bremming electron (we set E_0 = E_beam in this function)
            //  The momentum scale of the nucleus is ~1/fermi~-.139531GeV=139.531MeV
            bool unphysicalWideAngleBrem( const SimParticle *particle ) const;

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
            double energyCut_; //minimum energy for primary electron pre-target to keep event
            double pTCut_; //maximum pT for primary electron pre-target to keep event
            double lowReconEnergy_; //definitive low Recon E for determining if event is saved
            double lowPNEnergy_; //definitie low PN energy for determining if event is saved

            //Persistence Counters
            int lowReconLowPN_; //counter for low recon energy in a low PN event
            int skippedEvents_; //counter for number of events that were skipped

            //ROOT Histograms
            TH2F *h_ReconE_TaggerElecE; //recon energy vs energy of electron in last layer of tagger
            TH2F *h_ReconE_TaggerElecPT; //recon energy vs pT of electron in last layer of tagger
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
