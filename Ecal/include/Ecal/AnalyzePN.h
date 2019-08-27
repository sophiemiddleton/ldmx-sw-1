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

            //returns true if point is in ECAL box (doesn't check for hexagonal towers)
            //  assumes ECAL centered at (0,0) in xy plane
            bool isInEcal( const std::vector<double> &point ) const;

            //returns true if particle is considered as "going PN"
            //  checks if any children of particle has processType photonNuclear
            bool goesPN( const SimParticle *particle ) const;

            //Python Configuration Parameters
            std::string simParticlesCollName_; //name of collection for sim particles to analyze
            std::string simParticlesPassName_; //name of pass for sim particles
            double ecalXYWidth_; //width of ecal box in x and y directions
            double ecalFrontZ_; //starting z coordinate of ecal
            double ecalDepth_; //depth of ecal in z direction

            //Persistence Counters
            int numStartInEcal_; //number of SimParticles that started in ECAL
            int numChildPN_; //number of SimParticles who has one child with processType PN
            int numChildnoPN_; //number of SimParticles with one child PN and another not PN
            int numPN_; //number of SimParticles whose processType is PN
    };
}

#endif /* ECAL_ANALYZEPN_H */
