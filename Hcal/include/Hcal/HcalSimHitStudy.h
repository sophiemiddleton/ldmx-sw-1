/**
 * @file HcalSimHitStudy.h
 * @brief
 * @author
 */

#ifndef HCAL_HCALSIMHITSTUDY_H
#define HCAL_HCALSIMHITSTUDY_H

//LDMX Framework
#include "Event/EventDef.h"
#include "Framework/EventProcessor.h" //Needed to declare processor
#include "Framework/ParameterSet.h" // Needed to import parameters from configuration file

#include <cmath> //pow

namespace ldmx {
    
    /**
     * @class HcalSimHitStudy
     * @brief 
     */
    class HcalSimHitStudy : public ldmx::Analyzer {
        public:

            HcalSimHitStudy(const std::string& name, ldmx::Process& process) : ldmx::Analyzer(name, process) { }

            virtual void configure(const ldmx::ParameterSet& ps);

            virtual void analyze(const ldmx::Event& event);

            virtual void onFileOpen() { }

            virtual void onFileClose() { }

            virtual void onProcessStart(); 

            virtual void onProcessEnd();

        private:

            ///////////////////////////////////////
            // Python Config Options
            std::string hcalSimHitColl_; //* Name of Hcal Digis Collection
            double backZeroLayer_; //* Location of Z-plane of Zero'th Layer of Back HCAL
            double sideZeroLayer_; //* Location of plane of Zero'th Layer of Side HCAL
            double ecalFrontZ_; //* Location of Z-plane of front of ECAL

            //The first coordinate of all these histograms is the pdgID of the particle that caused the HcalSimHit
            TH2F* h_Side_Depth;
            TH2F* h_Side_Z;
            TH2F* h_Back_Z;
            TH2F* h_EnergyDep;
            TH3F* h_ZbyR;
    };
}

#endif /* HCAL_HCALSIMHITSTUDY_H */
