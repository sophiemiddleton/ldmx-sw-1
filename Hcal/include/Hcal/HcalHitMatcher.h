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
#include "TString.h"
#include "TClonesArray.h"
#include "TDatabasePDG.h"

//LDMX Framework
#include "Event/EventDef.h"
#include "Framework/EventProcessor.h" //Needed to declare processor
#include "Framework/ParameterSet.h" // Needed to import parameters from configuration file

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
             *      MinDepth_IncludeEventMaxPE  NA
             *      ecalFrontZ                  NA
             */
            virtual void configure(const ldmx::ParameterSet& ps);
            
            /**
             * Fills histograms and attempts to match hcal hits with the SimParticle
             * that caused them.
             */
            virtual void analyze(const ldmx::Event& event);

            /**
             * Finds histogram directory and initializes all of the histograms.
             */
            virtual void onProcessStart(); 
            
            /**
             * Prints out totals on numbers of hcal hits.
             */
            virtual void onProcessEnd();

        private:

            /**
             * Calculate total measured energy in the ECAL.
             * Fill histogram along the way.
             */
            double calculateEcalSummedEnergy( const TClonesArray* ecalHitCollection );

            /**
             * Filter ECAL Scoring Plane collection for particles leaving
             * ECAL towards HCAL
             * Fill histograms along the way.
             */
            void getParticlesLeavingEcalScoringPlane( const TClonesArray * ecalScoringPlaneHits , 
                    double ecalTotalEnergy , std::vector<SimTrackerHit *> hitsLeavingEcal );

            ///////////////////////////////
            // Python Config Options
            std::string EcalHitColl_; //* Name of Ecal Digis Collection
            std::string HcalHitColl_; //* Name of Hcal Digis Collection
            std::string EcalScoringPlane_; //* Name of Ecal Scoring Plane Hits Collection
            std::string HcalScoringPlane_; //* Name of Hcal Scoring Plane Hits Collection
            double minLayerEventMaxPE_; //* Minimum depth of hit in HCAL section to include in Event Max PE [layer index]
            double ecalFrontZ_; //* Location of Z-plane of front of ECAL [mm]

            /////////////////////////////
            // Persistent information
            long int numNonNoiseHits_; //* Number of Non-Noise Hcal Hits
            long int numUnMatchedHits_; //* Number of Hcal Hits not assigned at least one contributor
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

            //Event information - integrate to number of events
            TH1F* h_EcalSummedEnergy;
            TH2F* h_NumHcalHits;
            TH2F* h_NumHcalHits_Back;
            TH2F* h_NumHcalHits_Side;
            TH2F* h_NumParticles;
            TH2F* h_EventMaxPE_All;
            TH2F* h_EventMaxPE_Excluded;  //Excludes any HcalHit with layer < minLayer

            //SimTrackerHit - integrate to number of SimTrackerHits
            TH2F* h_Particle_ID; //ID particles crossing scoring plane
            TH2F* h_Particle_Energy; //Energy of particles crossing scoring plane
            TH2F* h_Particle_Kinetic; //Kinetic energy of particles crossing scoring plane

            //HcalHit - integrate to number of HcalHits
            TH2F* h_HcalHit_Depth_Side;
            TH2F* h_HcalHit_Depth_Side_byID;
            TH2F* h_HcalHit_Depth_Back;
            TH2F* h_HcalHit_Z_Side;
            TH2F* h_HcalHit_NContribs;
            TH2F* h_HcalHit_IDs;
            TH3F* h_HcalHit_ZbyR_All;
            TH2F* h_HcalHit_PE_All;

    };
}

#endif /* HCAL_HCALHITMATCHER_H */
