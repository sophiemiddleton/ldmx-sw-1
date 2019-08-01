/**
 * @file HcalHitMatcher.cxx
 * @brief The purpose of this analyzer is to study vetoes caused by activity in the Hcal using 
 *        Monte Carlo simulations.
 *        It extracts an array of sim particles and then matches each sim particle to a hit in the Hcal.
 *        Hcal hits and sim particles are matched spatially by finding the closest distance from a 
 *        sim particle's trajectory to a reconstructed Hcal hit.
 *        Plots and results are then tabulated in ROOT based on the sim particle and Hcal hit matches.
 *
 * @author Matthew Forsman , Tom Eichlersmith
 */

#include "Hcal/HcalHitMatcher.h"
#include "Event/HcalHit.h"
#include "Event/SimParticle.h"

#include "TDirectoryFile.h" //directories to organize histograms

namespace ldmx {

    void HcalHitMatcher::configure(const ldmx::ParameterSet& ps) { 
        
        EcalHitColl_ = ps.getString( "EcalHitCollectionName" , "ecalDigis" );
        HcalHitColl_ = ps.getString( "HcalHitCollectionName" , "hcalDigis" );
        EcalScoringPlane_ = ps.getString( "EcalScoringPlaneHitsName" , "EcalScoringPlaneHits" ); 
        HcalScoringPlane_ = ps.getString( "HcalScoringPlaneHitsName" , "HcalScoringPlaneHits" );

        maxMatchDist_ = ps.getDouble( "MaximumMatchDistance" , 150.0 );

        minR_EventMaxPE_ = ps.getDouble( "MinRadial_IncludeEventMaxPE" , 500.0 );
        minZ_EventMaxPE_ = ps.getDouble( "MinZDepth_IncludeEventMaxPE" , 1000.0 );

        useAllSimParticles_ = static_cast<bool>( ps.getInteger( "UseAllSimParticles" , 0 ) );

        return;
    }

    void HcalHitMatcher::analyze(const ldmx::Event& event) {

        numEvents_++;

        //---------- This section obtains a list of sim particles that cross the ecal scoring plane --------->
        const TClonesArray* ecalScoringPlaneHits = event.getCollection( EcalScoringPlane_ );
        const TClonesArray* simParticles = event.getCollection("SimParticles"); // side-effect is to make TRefs all valid

        std::vector<ldmx::SimTrackerHit*> simTrackerHits_ScorePlane;
    
//        printf( "%12i : %11s %7s %5s %20s\n" , simParticles->GetEntriesFast() , "SimParticle", "Energy" , "PDGID" , "Momentum" );
        for (int i = 0; i < ecalScoringPlaneHits->GetEntriesFast(); i++ ) {
            ldmx::SimTrackerHit* ecalSPH = (ldmx::SimTrackerHit*)(ecalScoringPlaneHits->At(i));

            int layerID = ecalSPH->getLayerID();
            std::vector<double> momentum = ecalSPH->getMomentum();

            //skip particles that are entering the ECAL (mostly incoming electron)
            bool isLeavingECAL = false;
            switch( layerID ) {
                case 1: //front - nothing - not near HCAL
                    isLeavingECAL = false;
                    break;
                case 2: //back - z needs to be positive
                    isLeavingECAL = (momentum[2] > 0);
                    break;
                case 3: //top - y needs to be postive
                    isLeavingECAL = (momentum[1] > 0);
                    break;
                case 4: //bottom - y needs to be negative
                    isLeavingECAL = (momentum[1] < 0);
                    break;
                case 5: //right - x needs to be negative
                    isLeavingECAL = (momentum[0] < 0);
                    break;
                case 6: //left - x needs to be positive
                    isLeavingECAL = (momentum[0] > 0);
                    break;
                default:
                    isLeavingECAL = false;
                    std::cerr << "[ Warning ] : HcalHitMatcher found a ECAL Scoring Plane Hit with layerID " << layerID << std::endl;
                    std::cerr << "    which is not one of the options (1 - 6)." << std::endl;
            }

            if ( isLeavingECAL ) {
                simTrackerHits_ScorePlane.push_back(ecalSPH);
                std::vector<double> momentum = ecalSPH->getMomentum();
//                printf( "SimTrackerHit: %11p %7.2f %5i (%5.1f,%5.1f,%5.1f)\n" , 
//                        ecalSPH->getSimParticle(), ecalSPH->getEnergy() , ecalSPH->getPdgID(), 
//                        momentum[0], momentum[1], momentum[2]
//                        );
            }
        }

        std::sort(simTrackerHits_ScorePlane.begin(), simTrackerHits_ScorePlane.end(), compSims);

        //SimParticles that cross Ecal Scoring Planes
        std::vector< ldmx::SimParticle* > simParticleCrossEcalSP;

        ldmx::SimParticle* lastP = 0; //sometimes multiple SP hits from same particle
        for ( std::vector<ldmx::SimTrackerHit*>::iterator it_simVec = simTrackerHits_ScorePlane.begin();
              it_simVec != simTrackerHits_ScorePlane.end(); ++it_simVec ) {

            ldmx::SimParticle* sP = (*it_simVec)->getSimParticle();
            if ( sP != lastP ) {
                lastP = sP;
                simParticleCrossEcalSP.push_back( sP );
            }

        }

        // Use all SimParticles instead of Scoring Plane ones
        if ( useAllSimParticles_ ) {
            simParticleCrossEcalSP.clear();
            for ( int i_SP = 0; i_SP < simParticles->GetEntriesFast(); i_SP++ ) {
                simParticleCrossEcalSP.push_back( dynamic_cast<ldmx::SimParticle*>(simParticles->At( i_SP )) );
            }
        }

        //---------- This section calculates the energy in the ECAL ---------------------------------------->
        //Then uses this energy to set standard deviation range

        const TClonesArray* ecalHitColl = event.getCollection( EcalHitColl_ ); 

        double ecalTotalEnergy = 0;
        for(int i=0; i < ecalHitColl->GetEntriesFast(); i++) {
            ldmx::EcalHit* ecalhit = (ldmx::EcalHit*)(ecalHitColl->At(i));
            if ( ! ecalhit->isNoise() ) { //Only add non-noise hits
                ecalTotalEnergy += ecalhit->getEnergy();
            }
        }

        //Bin event information
        h_Ecal_SummedEnergy->Fill( ecalTotalEnergy );
    
        h_NumParticles->Fill( ecalTotalEnergy , simParticleCrossEcalSP.size() );

        //Go through all SimParticles that crossed ECAL Scoring Plane
        //for ( const ldmx::SimParticle* simPart : simParticleCrossEcalSP ) {
        for ( const ldmx::SimTrackerHit* simPart : simTrackerHits_ScorePlane ) {
            int pdgID = simPart->getPdgID();
            double energy = simPart->getEnergy();
            std::vector<double> momentum = simPart->getMomentum();
            double kinetic_energy = 0.0;
            for ( double pi : momentum ) { kinetic_energy += pi*pi; }

            h_Particle_PDGID_All->Fill( ecalTotalEnergy , pdgID );

            h_Particle_Energy_All->Fill( ecalTotalEnergy , kinetic_energy );
        }

        //----This section matches HCal hits to sim particles and records results----->
        const TClonesArray* hcalHitColl = event.getCollection( HcalHitColl_ );

        float max_PE_of_event=0;
        float max_PE_of_event_excluded=0;
        int nHcalHits = 0;
        for(int i=0; i < hcalHitColl->GetEntriesFast(); i++) { //Begin loop over hcalhits array
            ldmx::HcalHit* hcalhit = (ldmx::HcalHit*)(hcalHitColl->At(i));
            
            if ( ! hcalhit->getNoise() ) { //Only analyze non-noise hits
                
                //hcalhit->Print();

                numNonNoiseHits_++;
                nHcalHits++;

                //---- Bin HcalHit information that does not depend on mathcin -------------------->
                
                double hcalhit_radialdist2 = pow(hcalhit->getX(), 2) + pow(hcalhit->getY(), 2);
                double hcalhit_radialdist = 0;
                //check to avoid a floating point error
                if(abs(hcalhit_radialdist2) > 1e-5) {
                    hcalhit_radialdist = sqrt(hcalhit_radialdist2);
                }

                h_HcalHit_ZbyR_All->Fill( ecalTotalEnergy , hcalhit->getZ(), hcalhit_radialdist);
                h_HcalHit_Z->Fill( ecalTotalEnergy , hcalhit->getZ());
                h_HcalHit_Time_All->Fill( ecalTotalEnergy , hcalhit->getTime());
                h_HcalHit_PE_All->Fill( ecalTotalEnergy , hcalhit->getPE());
                
                
                if(hcalhit->getTime() < 15.0)  {
                    h_HcalHit_PE_TimeLess15->Fill( ecalTotalEnergy , hcalhit->getPE());
                    h_HcalHit_ZbyR_TimeLess15->Fill( ecalTotalEnergy , hcalhit->getZ(), hcalhit_radialdist);
                } else if(hcalhit->getTime() > 40.0)  {
                    h_HcalHit_PE_TimeGreat40->Fill( ecalTotalEnergy , hcalhit->getPE());
                    h_HcalHit_ZbyR_TimeGreat40->Fill( ecalTotalEnergy , hcalhit->getZ(), hcalhit_radialdist);
                }
                
                if(hcalhit->getPE() > max_PE_of_event) {
                    max_PE_of_event=hcalhit->getPE();
                    if ( hcalhit->getZ() > minZ_EventMaxPE_ or hcalhit_radialdist > minR_EventMaxPE_ ) {
                        max_PE_of_event_excluded = hcalhit->getPE();
                    }
                }

                //---- Attempt to match this HcalHit to a SimParticle that cross the Ecal SP ----->

                //Iterate over all SimParticles that cross Ecal Scoring Plane to try to find a match
                //  for this HcalHit
                double new_dist=9999, dist=9998; //initializing distance variables
                const ldmx::SimParticle* matchedParticle = nullptr;
                for ( const ldmx::SimParticle* simPart : simParticleCrossEcalSP ) {

                    std::vector<double> simStart = simPart->getVertex();
                    std::vector<double> simEnd = simPart->getEndPoint();
        
                    TVector3 simStartT = TVector3(simStart[0], simStart[1], simStart[2]);
                    TVector3 simEndT = TVector3(simEnd[0], simEnd[1], simEnd[2]);
                    TVector3 hCalPoint = TVector3(hcalhit->getX(), hcalhit->getY(), hcalhit->getZ());
                    
                    new_dist = point_line_distance(simStartT, simEndT, hCalPoint);
                    
                    h_Particle_HitDistance_All->Fill( ecalTotalEnergy , new_dist);
        
                    if( false ) { //simStart[2]<10.0 and simPart->getEnergy()>3000.0) {
                        //discarding original electron
                        //DO NOTHING (skip this sim particle)
                    } else if(new_dist < dist) {
                        dist = new_dist; //Distance to matched particle
                        matchedParticle = simPart;
                    }
                    
                } //iterate over sim particles to match one to current hcal hit

                //---- Bin HcalHit/SimParticle information for successfully matched hits --------->

                if( matchedParticle and dist <= maxMatchDist_ ) {
                
                    numMatchedHits_++;

                    h_Particle_HitDistance_Matched->Fill( ecalTotalEnergy ,  dist );
        
                    h_HcalHit_Time_Matched_All->Fill( ecalTotalEnergy , hcalhit->getTime());
                    
                    int pdgID = matchedParticle->getPdgID();

                    h_Particle_PDGID_Matched->Fill( ecalTotalEnergy , pdgID);
        
                    double part_hcalhit_timeDiff = (hcalhit->getTime()) - (matchedParticle->getTime());
                    
                    h_HcalHit_Time_Matched_Tdif->Fill( ecalTotalEnergy , part_hcalhit_timeDiff);

                    if(part_hcalhit_timeDiff < 15.0)  {
                        h_HcalHit_PE_Matched_TdifLess15->Fill( ecalTotalEnergy , hcalhit->getPE());
                        h_HcalHit_ZbyR_Matched_TdifLess15->Fill( ecalTotalEnergy , hcalhit->getZ(), hcalhit_radialdist);
                    } else if(part_hcalhit_timeDiff > 40.0)  {
                        h_HcalHit_PE_Matched_TdifGreat40->Fill( ecalTotalEnergy , hcalhit->getPE());
                        h_HcalHit_ZbyR_Matched_TdifGreat40->Fill( ecalTotalEnergy , hcalhit->getZ(), hcalhit_radialdist);
                    }
                    
                    //protons or neutrons (nucleons) 
                    if( pdgID==2112 or pdgID==2212 ) { 
                        h_HcalHit_Time_Matched_Nucleons->Fill( ecalTotalEnergy , matchedParticle->getTime());
                    }
                    
                    h_Particle_Energy_Matched->Fill( ecalTotalEnergy , matchedParticle->getEnergy());
        
                    switch(pdgID) {
                        case 11:
                            h_HcalHit_ZbyR_Matched_Electron->Fill( ecalTotalEnergy , hcalhit->getZ(), hcalhit_radialdist); 
                            break;
                        case 22:
                            h_HcalHit_ZbyR_Matched_Photon->Fill( ecalTotalEnergy , hcalhit->getZ(), hcalhit_radialdist); 
                            break;
                        case 2112:
                            h_HcalHit_ZbyR_Matched_Neutron->Fill( ecalTotalEnergy , hcalhit->getZ(), hcalhit_radialdist); 
                            break;
                        default:
                            h_HcalHit_ZbyR_Matched_Other->Fill( ecalTotalEnergy , hcalhit->getZ(), hcalhit_radialdist); 
                            break;
                    }

                } else {

                    h_HcalHit_ZbyR_Unmatched->Fill( ecalTotalEnergy , hcalhit->getZ(), hcalhit_radialdist);

                } //matched or unmatched
    
            } // if not a noise hit

        }//End loop over hcalhits array

        // maximum PE in hcal hits for the event
        h_EventMaxPE_All->Fill( ecalTotalEnergy , max_PE_of_event );
        h_EventMaxPE_Excluded->Fill( ecalTotalEnergy , max_PE_of_event_excluded );
        h_NumHcalHits->Fill( ecalTotalEnergy , nHcalHits );

        return;
    } //analyze

   
    bool HcalHitMatcher::compSimsP(const SimTrackerHit* a, const SimTrackerHit* b) {
        std::vector<double> paVec = a->getMomentum();
        std::vector<double> pbVec = b->getMomentum();
    
        double pa2 = pow(paVec[0],2)+pow(paVec[1],2)+pow(paVec[2],2);
        double pb2 = pow(pbVec[0],2)+pow(pbVec[1],2)+pow(pbVec[2],2);
    
        return pa2 > pb2;
    }

    bool HcalHitMatcher::compSims(const SimTrackerHit* a, const SimTrackerHit* b) {

        if (a->getSimParticle() == b->getSimParticle()) {
             return compSimsP(a,b);
        } else {
             return a->getSimParticle() < b->getSimParticle();
        }
    }

    //This function finds the minimum distance between a line segment and a point in 3D space
    double HcalHitMatcher::point_line_distance(TVector3 v, TVector3 w, TVector3 p)  {
        
        //Define 2 line segments from point v to w and from point v to p
        TVector3 vw = TVector3(w.x()-v.x(), w.y()-v.y(), w.z()-v.z());
        TVector3 vp = TVector3(v.x()-p.x(), v.y()-p.y(), v.z()-p.z());
    
        //Define both line segment's magnitude squared
        const double L2vw = pow(vw.Mag(), 2);
        const double L2vp = pow(vp.Mag(), 2);
    
        //Define a parameter t clamped between 0 and 1 so that the distance is never measured from 
        //beyond the endpoints of the line segment
        const double t = std::max(0.0, std::min(1.0, -1*vp.Dot(vw)/L2vw));
        //const double t = std::max(0.0, -1*vp.Dot(vw)/L2vw);//matt was using this
    
        //Calculate the minimum distance squared
        double d2 = L2vp + 2.0*t*vp.Dot(vw) + pow(t,2)*L2vw;
    
        //Return minimum distance and catch floating point errors near zero
        if(abs(d2) < 1e-5)
            return 0;
        else
            return sqrt(d2);
    }

    void HcalHitMatcher::onProcessStart() {
        
        numNonNoiseHits_ = 0;
        numMatchedHits_ = 0;
        numEvents_ = 0;

        // Make directory tree to organize histograms in
        TDirectory* d_base = getHistoDirectory();

        h_Ecal_SummedEnergy = new TH1D(
                "Ecal_SummedEnergy",
                "Ecal Summed Energy;Energy [MeV] (10MeV bin width);Count",
                800,0,8000);//10MeV bins

        h_NumHcalHits = new TH2D(
                "NumHcalHits",
                ";EcalSummedEnergy;Number of HcalHits per Event;Count",
                800,0,8000,
                50,0,50);

        h_NumParticles = new TH2D(
                "NumParticles" ,
                ";EcalSummedEnergy;Number of SimParticles that Crossed the ECAL Scoring Plane;Count",
                800,0,8000,
                10,0,10);

        h_EventMaxPE_All = new TH2D(
                "EventMaxPE_All",
                ";EcalSummedEnergy;Maximum PE for all HcalHits in Event;Count",
                800,0,8000,
                50,0,500);

        char title[200];
        sprintf( title , ";EcalSummedEnergy;Maximum PE for HcalHits with R > %.1f mm or Z > %.1f mm; Count" ,
                minR_EventMaxPE_ , minZ_EventMaxPE_ );
        h_EventMaxPE_Excluded = new TH2D(
                "EventMaxPE_Excluded",
                title,
                800,0,8000,
                50,0,500);
        
        h_Particle_PDGID_All = new TH2D(
                "Particle_PDGID_All",
                ";EcalSummedEnergy;PDG ID of SimParticle Crossing ECAL Scoring Plane;Count",
                800,0,8000,
                10000,-5000,5000);

        h_Particle_PDGID_Matched = new TH2D(
                "Particle_PDGID_Matched",
                ";EcalSummedEnergy;PDG ID of SimParticle Matched to HcalHit;Count",
                800,0,8000,
                10000,-5000,5000);

        h_Particle_HitDistance_All = new TH2D(
               "Particle_HitDistance_All",
               ";EcalSummedEnergy;Distance between SimParticle and HcalHit (Any Pair) [mm];Count",
               800,0,8000,
               200, 0, 2000);
       
        h_Particle_HitDistance_Matched = new TH2D(
               "Particle_HitDistance_Matched", 
               ";EcalSummedEnergy;Distance between SimParticle and HcalHit when matched [mm];Count", 
               800,0,8000,
               int(maxMatchDist_/10), 0, maxMatchDist_+10.0 );
       
        h_Particle_Energy_All = new TH2D(
               "Particle_Energy_All",
               ";EcalSummedEnergy;SimParticle Energy [MeV];Count",
               800,0,8000,
               41,0,4100);

        h_Particle_Energy_Matched = new TH2D(
               "Particle_Energy_Matched",
               ";EcalSummedEnergy;Energy of SimParticles matched to HcalHit [MeV];Count",
               800,0,8000,
               41,0,4100);

        h_HcalHit_Z = new TH2D(
               "HcalHit_Z",
               ";EcalSummedEnergy;Z depth of HcalHits [mm];Count",
               800,0,8000,
               100, 0, 5000);

        h_HcalHit_ZbyR_All = new TH3D(
               "HcalHit_ZbyR_All", 
               "All Hcal Hit Locations;EcalSummedEnergy;Z depth [mm];radial distance from z-axis [mm]",
               800,0,8000,
               100,0,5000,
               50,0,2200);

        h_HcalHit_ZbyR_Unmatched = new TH3D(
               "HcalHit_ZbyR_Unmatched", 
               "Hcal unmatched hit locations;EcalSummedEnergy;Z depth [mm];radial distance from z-axis [mm]",
               800,0,8000,
               100,0,5000,
               50,0,2200);

        h_HcalHit_ZbyR_TimeLess15 = new TH3D(
               "HcalHit_ZbyR_TimeLess15",
               "HcalHits with Time < 15ns locations;EcalSummedEnergy;Z depth [mm];radial distance from z-axis [mm]",
               800,0,8000,
               100,0,5000,
               50,0,2200);

        h_HcalHit_ZbyR_TimeGreat40 = new TH3D(
               "HcalHit_ZbyR_TimeGreat40",
               "HcalHits with Time > 40ns locations;EcalSummedEnergy;Z depth [mm];radial distance from z-axis [mm]",
               800,0,8000,
               100,0,5000,
               50,0,2200);

        h_HcalHit_ZbyR_Matched_Photon = new TH3D(
               "HcalHit_ZbyR_Matched_Photon", 
               "Hcal photon hit locations;EcalSummedEnergy;Z depth [mm];radial distance from z-axis [mm]",
               800,0,8000,
               100,0,5000,
               50,0,2200);

        h_HcalHit_ZbyR_Matched_Electron = new TH3D(
               "HcalHit_ZbyR_Matched_Electron", 
               "Hcal electron hit locations;EcalSummedEnergy;Z depth [mm];radial distance from z-axis [mm]",
               800,0,8000,
               100,0,5000,
               50,0,2200);

        h_HcalHit_ZbyR_Matched_Neutron = new TH3D(
               "HcalHit_ZbyR_Matched_Neutron", 
               "Hcal neutron hit locations;EcalSummedEnergy;Z depth [mm];radial distance from z-axis [mm]",
               800,0,8000,
               100,0,5000,
               50,0,2200);

        h_HcalHit_ZbyR_Matched_Other = new TH3D(
               "HcalHit_ZbyR_Matched_Other", 
               "Hcal other particle hit locations;EcalSummedEnergy;Z depth [mm];radial distance from z-axis [mm]",
               800,0,8000,
               100,0,5000,
               50,0,2200);

        h_HcalHit_ZbyR_Matched_TdifLess15 = new TH3D(
               "HcalHit_ZbyR_Matched_TdifLess15",
               "Matched HcalHit location with time dif < 15ns;EcalSummedEnergy;Z depth [mm];radial distance from z-axis [mm]",
               800,0,8000,
               100,0,5000,
               50,0,2200);

        h_HcalHit_ZbyR_Matched_TdifGreat40 = new TH3D(
               "HcalHit_ZbyR_Matched_TdifGreat40",
               "Matched HcalHit location with time dif > 40ns;EcalSummedEnergy;Z depth [mm];radial distance from z-axis [mm]",
               800,0,8000,
               100,0,5000,
               50,0,2200);

        h_HcalHit_PE_All = new TH2D(
               "HcalHit_PE_All",
               ";EcalSummedEnergy;PEs of all HcalHits;Count",
               800,0,8000,
               50,0,500);

        h_HcalHit_PE_TimeLess15 = new TH2D(
               "HcalHit_PE_TimeLess15",
              ";EcalSummedEnergy;PEs of HcalHits with time < 15 ns;Count",
               800,0,8000,
               50,0,500);

        h_HcalHit_PE_TimeGreat40 = new TH2D(
               "HcalHit_PE_TimeGreat40",
               ";EcalSummedEnergy;PEs of HcalHits with time > 40 ns;Count",
               800,0,8000,
               50,0,500);

        h_HcalHit_PE_Matched_TdifLess15 = new TH2D(
               "HcalHit_PE_Matched_TdifLess15",
               ";EcalSummedEnergy;PEs of Matched Hits with time difference < 15 ns;Count",
               800,0,8000,
               50,0,500);

        h_HcalHit_PE_Matched_TdifGreat40 = new TH2D(
               "HcalHit_PE_Matched_TdifGreat40",
               ";EcalSummedEnergy;PEs of Matched Hits with time difference > 40 ns;Count",
               800,0,8000,
               50,0,500);

        h_HcalHit_Time_All = new TH2D(
               "HcalHit_Time_All",
               ";EcalSummedEnergy;Time of all HcalHits [ns];Count",
               800,0,8000,
               50,0,500);

        h_HcalHit_Time_Matched_All = new TH2D(
               "HcalHit_Time_Matched_All", 
               ";EcalSummedEnergy;Time of Matched HcalHits [ns];Number of particles created", 
               800,0,8000,
               50, 0, 500);

        h_HcalHit_Time_Matched_Nucleons = new TH2D(
               "HcalHit_Time_Matched_Nucleons",
               ";EcalSummedEnergy;Time of HcalHits matched to Nucelons [ns];Number of Nucleons created", 
               800,0,8000,
               50, 0, 500);
       
        h_HcalHit_Time_Matched_Tdif = new TH2D(
               "HcalHit_Time_Matched_Tdif",
               ";EcalSummedEnergy;Time Difference Between SimParticle and matched HcalHit [ns];Count",
               800,0,8000,
               50,0,200);
    
        return;
    } //onProcessStart
    
    void HcalHitMatcher::onProcessEnd() {
        
        double hitRate, matchRate;
        {
            //temporary variables for calculating rates
            double numerator = numNonNoiseHits_;
            double denominator = numEvents_;
            hitRate = numerator/denominator;

            numerator = numMatchedHits_;
            denominator = numNonNoiseHits_;
            matchRate = numerator / denominator;
        }

        printf( "Number of Events         : %i\n" , numEvents_ );
        printf( "Number of Non Noise Hits : %i\n" , numNonNoiseHits_ );
        printf( "Number of Matched Hits   : %i\n" , numMatchedHits_ );
        printf( "Hit Rate (hits/events)   : %f\n" , hitRate );
        printf( "Match Rate (matches/hits): %f\n" , matchRate );

        return;
    }

} //ldmx namespace

DECLARE_ANALYZER_NS(ldmx, HcalHitMatcher);
