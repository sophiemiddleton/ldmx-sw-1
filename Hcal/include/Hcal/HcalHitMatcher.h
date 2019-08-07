/**
 * @file HcalHitMatcher.h
 * @brief
 * @author Matthew Forsman
 * @author Tom Eichlersmith
 */
#ifndef HCAL_HCALHITMATCHER_H
#define HCAL_HCALHITMATCHER_H

#include "Event/SimTrackerHit.h"
#include "Framework/EventProcessor.h"
#include "Framework/ParameterSet.h"
#include "Event/CalorimeterHit.h"

#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <string>
#include <map>

#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TClonesArray.h"
#include "TVector3.h"
#include "TDatabasePDG.h"

//LDMX Framework
#include "Event/Event.h"
#include "Framework/EventProcessor.h" //Needed to declare processor
#include "Framework/ParameterSet.h" // Needed to import parameters from configuration file

class SimTrackerHit;

namespace ldmx {
    
    /**
     * @class HcalHitMatcher
     * @brief 
     */
    class HcalHitMatcher : public ldmx::Analyzer {
        public:

            HcalHitMatcher(const std::string& name, ldmx::Process& process) : ldmx::Analyzer(name, process) {}

            /**
             * Gets options from ParameterSet class.
             *      Option                      Default                 
             *      EcalHitCollectionName       ecalDigis               
             *      HcalHitCollectionName       hcalDigis               
             *      EcalScoringPlaneHitsName    EcalScoringPlaneHits    
             *      HcalScoringPlaneHitsName    HcalScoringPlaneHits
             *      MaximumMatchDistance        150.0 (Maximum distance allowed for a match in mm)
             *      MinRadial_IncludeEventMaxPE 500.0 (Minimum Radial distance allowed for maximum PE calculation)
             *      MinZDepth_IncludeEventMaxPE 1000.0 (Minimum Z Depth allowed for maximum PE calculation)
             */
            virtual void configure(const ldmx::ParameterSet& ps);
            
            /**
             * Fills histograms and attempts to match hcal hits with the SimParticle
             * that caused them.
             */
            virtual void analyze(const ldmx::Event& event);
    
            /**
             * Calculate the distance between the line segment from v to w and the point p.
             */
            double point_line_distance(TVector3 v, TVector3 w, TVector3 p);
    
            virtual void onFileOpen() { }

            virtual void onFileClose() { }
        
            /**
             * Finds histogram directory and initializes all of the histograms.
             */
            virtual void onProcessStart(); 
            
            /**
             * Prints out totals on numbers of hcal hits.
             */
            virtual void onProcessEnd();

        private:
    
            ///////////////////////////////
            // Python Config Options
            std::string EcalHitColl_; //* Name of Ecal Digis Collection
            std::string HcalHitColl_; //* Name of Hcal Digis Collection
            std::string EcalScoringPlane_; //* Name of Ecal Scoring Plane Hits Collection
            std::string HcalScoringPlane_; //* Name of Hcal Scoring Plane Hits Collection
            double maxMatchDist_; //* Maximum distance (mm) between SimParticle and Hit to allow for a match
            double minDepth_EventMaxPE_; //* Minimum depth of hit in HCAL section to include in Event Max PE
            double backZeroLayer_; //* Location of Z-plane of Zero'th Layer of Back HCAL
            double sideZeroLayer_; //* Location of plane of Zero'th Layer of Side HCAL
            double ecalFront_; //* Location of Z-plane of front of ECAL

            /////////////////////////////
            // Persistent information
            long int numNonNoiseHits_; //* Number of Non-Noise Hcal Hits
            long int numMatchedHits_; //* Number of Hcal Hits matched to a sim particle
            long int numEvents_; //* Number of events analyzed
            std::map< int , long int > numParticles_; //* Number of particles corresponding to each PDG ID
            TDatabasePDG databasePDG_; //* ROOT database with PDG information
        
            ///////////////////////////
            // Histograms

            /**
             * The first index/coordinate in all of these histograms is the total (non-noise)
             * energy deposited in the ECAL.
             *
             * This makes it possible to look at specific ranges of ECAL deposited energy
             * when analyzing the results without having to re-run everytime ranges we are
             * interested in change.
             */

            //Event information (i.e. One Entry per Event)
            TH1D* h_Ecal_SummedEnergy;
            TH2D* h_NumHcalHits;
            TH2D* h_NumHcalHits_Back;
            TH2D* h_NumHcalHits_Side;
            TH2D* h_NumParticles;
            TH2D* h_EventMaxPE_All;
            TH2D* h_EventMaxPE_Excluded;  //Excludes any HcalHit with depth < minDepth

            //SimTrackerHit
            TH2D* h_Particle_ID;
//            TH2D* h_Particle_HitDistance_All; //Distance between Particles and HcalHits
//            TH2D* h_Particle_HitDistance_Matched; //Distance between Particles and HcalHits
            TH2D* h_Particle_Energy_All; //All Particle energies
            TH2D* h_Particle_Kinetic_All; //All Particle kinetic energies
//            TH2D* h_Particle_Energy_Matched; //Matched Particle energies

            //Position of HcalHits
            TH2D* h_HcalHit_Depth_Side;
            TH2D* h_HcalHit_Z_Side;
            TH2D* h_HcalHit_Z_Back;
            TH3D* h_HcalHit_ZbyR_All;
//            TH3D* h_HcalHit_ZbyR_Unmatched;
//            TH3D* h_HcalHit_ZbyR_TimeLess15;
//            TH3D* h_HcalHit_ZbyR_TimeGreat40;
//            TH3D* h_HcalHit_ZbyR_Matched_Photon;
//            TH3D* h_HcalHit_ZbyR_Matched_Electron;
//            TH3D* h_HcalHit_ZbyR_Matched_Neutron;
//            TH3D* h_HcalHit_ZbyR_Matched_Other;
//            TH3D* h_HcalHit_ZbyR_Matched_TdifLess15;
//            TH3D* h_HcalHit_ZbyR_Matched_TdifGreat40;
            
            //PEs of HcalHit
            TH2D* h_HcalHit_PE_All;
//            TH2D* h_HcalHit_PE_TimeLess15;
//            TH2D* h_HcalHit_PE_TimeGreat40;
//            TH2D* h_HcalHit_PE_Matched_TdifLess15;
//            TH2D* h_HcalHit_PE_Matched_TdifGreat40;

            //Time of HcalHit
            TH2D* h_HcalHit_Time_All;
//            TH2D* h_HcalHit_Time_Matched_All;
//            TH2D* h_HcalHit_Time_Matched_Nucleons;
//            TH2D* h_HcalHit_Time_Matched_Tdif;

    };
}

#endif /* HCAL_HCALHITMATCHER_H */
