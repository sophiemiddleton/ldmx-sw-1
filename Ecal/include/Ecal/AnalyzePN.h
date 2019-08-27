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

            //totals the non-noise reconstructed energy in ECAL
            double calculateReconEnergy( const TClonesArray *ecalHitColl ) const;

            //returns true if particle is classified as mid-shower PN
            //  checks if particle's starting point is in ecal and if it "goes PN"
            bool isMidShowerPN( const SimParticle *particle ) const;

            //returns true if point is in ECAL box (doesn't check for hexagonal towers)
            //  assumes ECAL centered at (0,0) in xy plane
            bool isInEcal( const std::vector<double> &point ) const;

            //returns true if particle is considered as "going PN"
            //  checks if any children of particle has processType photonNuclear
            bool goesPN( const SimParticle *particle ) const;

            //Python Configuration Parameters
            std::string simParticlesCollName_; //name of collection for sim particles to analyze
            std::string simParticlesPassName_; //name of pass for sim particles
            std::string ecalDigiCollName_; //name of collection to calculate ecal digis
            std::string ecalDigiPassName_; //name of pass to get ecal digis collection
            double ecalXYWidth_; //width of ecal box in x and y directions
            double ecalFrontZ_; //starting z coordinate of ecal
            double ecalDepth_; //depth of ecal in z direction

            //Persistence Counters
            
            //ROOT Histograms
            TH2F *h_ReconE_HardestPN; //recon energy vs energy of hardest photon going PN
    };
}

#endif /* ECAL_ANALYZEPN_H */
