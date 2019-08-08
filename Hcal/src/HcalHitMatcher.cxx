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
#include "Event/EcalHit.h"

#include "TDirectoryFile.h" //directories to organize histograms

namespace ldmx {

    void HcalHitMatcher::configure(const ldmx::ParameterSet& ps) { 
        
        EcalHitColl_ = ps.getString( "EcalHitCollectionName" , "ecalDigis" );
        HcalHitColl_ = ps.getString( "HcalHitCollectionName" , "hcalDigis" );
        EcalScoringPlane_ = ps.getString( "EcalScoringPlaneHitsName" , "EcalScoringPlaneHits" ); 
        HcalScoringPlane_ = ps.getString( "HcalScoringPlaneHitsName" , "HcalScoringPlaneHits" );

        maxMatchDist_ = ps.getDouble( "MaximumMatchDistance" , 150.0 );

        minDepth_EventMaxPE_ = ps.getDouble( "MinDepth_IncludeEventMaxPE" );

        backZeroLayer_ = ps.getDouble( "backZeroLayer" );
        sideZeroLayer_ = ps.getDouble( "sideZeroLayer" );
        ecalFrontZ_ = ps.getDouble( "ecalFrontZ" );

        return;
    }

    void HcalHitMatcher::analyze(const ldmx::Event& event) {

        numEvents_++;

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
        
        //---------- This section obtains a list of sim particles that cross the ecal scoring plane --------->
        const TClonesArray* ecalScoringPlaneHits = event.getCollection( EcalScoringPlane_ );
        
        std::vector<ldmx::SimTrackerHit*> simTrackerHits_LeavingScorePlane;
    
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
                simTrackerHits_LeavingScorePlane.push_back(ecalSPH);
                int pdgID = ecalSPH->getPdgID();
                double mass = 0;
                if ( databasePDG_.GetParticle( pdgID ) ) { 
                    mass = databasePDG_.GetParticle( pdgID )->Mass() * 1000; //returns in GeV --> convert to MeV
                } else {
                    //Geant4 uses non-PDG IDs for composite particles (like nuclei and ions)
                    //  Need to look these up manually
                    if ( pdgID == 1000010020 ) {
                        //deuteron - proton and neutron
                        mass = 938.272 + 939.565;
                    }
                }

                double energy = ecalSPH->getEnergy();
                double kinetic = energy - mass;

                if ( numParticles_.find( pdgID ) == numParticles_.end() ) {
                    //no particles with this PDG found yet
                    numParticles_[ pdgID ] = 1;
                } else {
                    //particles with this PDG have been found
                    numParticles_[ pdgID ] ++;
                }

                if ( abs(pdgID) != 12 and abs(pdgID) != 14 ) {
                    //no neutrinos
                    h_Particle_ID->Fill( ecalTotalEnergy , std::to_string( pdgID ).c_str() , 1. );
                }

                h_Particle_Energy_All->Fill( ecalTotalEnergy , energy );
                h_Particle_Kinetic_All->Fill( ecalTotalEnergy , kinetic );

            }
        }

        h_NumParticles->Fill( ecalTotalEnergy , simTrackerHits_LeavingScorePlane.size() );

        //----This section matches HCal hits to sim particles and records results----->
        const TClonesArray* hcalHitColl = event.getCollection( HcalHitColl_ );

        float max_PE_of_event=0;
        float max_PE_of_event_excluded=0;
        int nBackHcalHits = 0;
        int nSideHcalHits = 0;
        for(int i=0; i < hcalHitColl->GetEntriesFast(); i++) { //Begin loop over hcalhits array
            ldmx::HcalHit* hcalhit = (ldmx::HcalHit*)(hcalHitColl->At(i));
            
            if ( ! hcalhit->getNoise() ) { //Only analyze non-noise hits
                
                numNonNoiseHits_++;

                //---- Bin HcalHit information that does not depend on mathcin -------------------->
                
                double hcalhit_radialdist = sqrt( pow(hcalhit->getX(), 2) + pow(hcalhit->getY(), 2) );

                h_HcalHit_ZbyR_All->Fill( ecalTotalEnergy , hcalhit->getZ(), hcalhit_radialdist);

                int section = hcalhit->getSection();
                int layer = hcalhit->getLayer();
                if ( section == 0 ) {
                    h_HcalHit_Depth_Back->Fill( ecalTotalEnergy , layer );
                    nBackHcalHits++;
                } else if ( seciton > 0 and section < 5 ) {
                    h_HcalHit_Z_Side->Fill( ecalTotalEnergy , hcalhit->getZ()-ecalFrontZ_ );
                    h_HcalHit_Depth_Side->Fill( ecalTotalEnergy , layer );
                    nSideHcalHits++;
                } else {
                    std::cerr << "[ Warning ] : HcalHitMatcher::analyze - found HcalSection " << section
                        << " that is not in the correct range." << std::endl;
                }

                h_HcalHit_PE_All->Fill( ecalTotalEnergy , hcalhit->getPE());
                
                float pe = hcalhit->getPE();
                if( pe > max_PE_of_event ) {
                    max_PE_of_event = pe;
                    if ( depth > minDepth_EventMaxPE_ ) {
                        max_PE_of_event_excluded = pe;
                    }
                }

                //---- Attempt to match this HcalHit to a Particle that cross the Ecal SP ----->

                //Iterate over all Particles that cross Ecal Scoring Plane to try to find a match
                //  for this HcalHit
                double dist = maxMatchDist_ + 10.0; //initializing distance variable to be greater than matching distance
                const ldmx::SimTrackerHit* matchedParticle = nullptr;
                for ( const ldmx::SimTrackerHit * scorePlaneHit : simTrackerHits_LeavingScorePlane ) {

                    std::vector<float>  simPos      = scorePlaneHit->getPosition();
                    std::vector<double> simMomentum = scorePlaneHit->getMomentum();
        
                    TVector3 rayStart  = TVector3( simPos[0] , simPos[1] , simPos[2] );
                    TVector3 rayDir    = TVector3( simMomentum[0] , simMomentum[1] , simMomentum[2] );
                    TVector3 hcalPoint = TVector3( hcalhit->getX(), hcalhit->getY(), hcalhit->getZ());
                    
                    double new_dist = pointRayDistance( rayStart , rayDir , hcalPoint );
                    
                    h_Particle_HitDistance_All->Fill( ecalTotalEnergy , new_dist );
        
                    if(new_dist < dist) {
                        dist = new_dist; //Distance to matched particle
                        matchedParticle = scorePlaneHit;
                    }

                } //iterate over sim particles to match one to current hcal hit

                //---- Bin HcalHit/Particle information for successfully matched hits --------->

                if( dist <= maxMatchDist_ ) {
                
                    int pdgID = matchedParticle->getPdgID();
                    double mass = 0;
                    if ( databasePDG_.GetParticle( pdgID ) ) { 
                        mass = databasePDG_.GetParticle( pdgID )->Mass() * 1000; //returns in GeV --> convert to MeV
                    } else {
                        //Geant4 uses non-PDG IDs for composite particles (like nuclei and ions)
                        //  Need to look these up manually
                        if ( pdgID == 1000010020 ) {
                            //deuteron - proton and neutron
                            mass = 938.272 + 939.565;
                        }
                    }
    
                    double energy = matchedParticle->getEnergy();
                    double kinetic = energy - mass;

                    numMatchedHits_++;

                    h_Particle_HitDistance_Matched->Fill( ecalTotalEnergy , dist    );
                    h_Particle_Energy_Matched     ->Fill( ecalTotalEnergy , energy  );
                    h_Particle_Kinetic_Matched    ->Fill( ecalTotalEnergy , kinetic );
        
                    switch(pdgID) {
                        case 11:
                            h_HcalHit_ZbyR_Matched_Electron->Fill( ecalTotalEnergy , hcalhit->getZ(), hcalhit_radialdist); 
                            break;
                        case 22:
                            h_HcalHit_ZbyR_Matched_Photon  ->Fill( ecalTotalEnergy , hcalhit->getZ(), hcalhit_radialdist); 
                            break;
                        case 2112:
                            h_HcalHit_ZbyR_Matched_Neutron ->Fill( ecalTotalEnergy , hcalhit->getZ(), hcalhit_radialdist); 
                            break;
                        default:
                            h_HcalHit_ZbyR_Matched_Other   ->Fill( ecalTotalEnergy , hcalhit->getZ(), hcalhit_radialdist); 
                            break;
                    }

                } else {

                    h_HcalHit_ZbyR_Unmatched->Fill( ecalTotalEnergy , hcalhit->getZ(), hcalhit_radialdist);

                } //matched or unmatched
    
            } // if not a noise hit

        }//End loop over hcalhits array

        // maximum PE in hcal hits for the event
        h_EventMaxPE_All     ->Fill( ecalTotalEnergy , max_PE_of_event );
        h_EventMaxPE_Excluded->Fill( ecalTotalEnergy , max_PE_of_event_excluded );

        // number of hcal hits for the event
        h_NumHcalHits     ->Fill( ecalTotalEnergy , nBackHcalHits+nSideHcalHits );
        h_NumHcalHits_Back->Fill( ecalTotalEnergy , nBackHcalHits );
        h_NumHcalHits_Side->Fill( ecalTotalEnergy , nSideHcalHits );

        return;
    } //analyze

    void HcalHitMatcher::onProcessStart() {
        
        //Readin default database installed with ROOTSYS
        databasePDG_.ReadPDGTable();

        numNonNoiseHits_ = 0;
        numMatchedHits_ = 0;
        numEvents_ = 0;

        // Make directory tree to organize histograms in
        TDirectory* d_base = getHistoDirectory();

        h_EcalSummedEnergy = new TH1F(
                "EcalSummedEnergy",
                "Ecal Summed Energy;Total Measured Energy in ECAL [MeV];Count",
                800,0,8000); //10 MeV Bins

        h_NumHcalHits = new TH2F(
                "NumHcalHits",
                ";EcalSummedEnergy;Number of HcalHits per Event;Count",
                800,0,8000,
                50,0,50);

        h_NumHcalHits_Back = new TH2F(
                "NumHcalHits_Back",
                ";EcalSummedEnergy;Number of Hits in Back HCAL per Event;Count",
                800,0,8000,
                50,0,50);

        h_NumHcalHits_Side = new TH2F(
                "NumHcalHits_Side",
                ";EcalSummedEnergy;Number of Hits in Side HCAL per Event;Count",
                800,0,8000,
                50,0,50);

        h_NumParticles = new TH2F(
                "NumParticles" ,
                ";EcalSummedEnergy;Number of Particles that Crossed the ECAL Scoring Plane;Count",
                800,0,8000,
                10,0,10);

        h_EventMaxPE_All = new TH2F(
                "EventMaxPE_All",
                ";EcalSummedEnergy;Maximum PE for all HcalHits in Event;Count",
                800,0,8000,
                500,0,500);

        char title[200];
        sprintf( title , ";EcalSummedEnergy;Maximum PE for Hits with Depth > %.1f mm; Count" ,
                minDepth_EventMaxPE_ );
        h_EventMaxPE_Excluded = new TH2F(
                "EventMaxPE_Excluded",
                title,
                800,0,8000,
                500,0,500);
        
        //add in bins of known particles
        std::vector<std::string> knownPDGs = { "22" , "11" , "-11" , "13" , "-13", 
            "2112" , "2212" , "211", "-211" , "130", "321" , "1000010020" };
        h_Particle_ID = new TH2F(
                "Particle_ID",
                ";EcalSummedEnergy;Particle Crossing ECAL Scoring Plane;Count",
                800,0,8000,
                knownPDGs.size(),0, knownPDGs.size() );
        for ( int ibin = 1; ibin < knownPDGs.size()+1; ibin++ ) {
            h_Particle_ID->GetYaxis()->SetBinLabel( ibin , knownPDGs.at(ibin-1).c_str() );
        }

        h_Particle_HitDistance_All = new TH2F(
               "Particle_HitDistance_All",
               ";EcalSummedEnergy;Distance between Particle and HcalHit (Any Pair) [mm];Count",
               800,0,8000,
               200, 0, 2000);
       
        h_Particle_HitDistance_Matched = new TH2F(
               "Particle_HitDistance_Matched", 
               ";EcalSummedEnergy;Distance between Particle and HcalHit when matched [mm];Count", 
               800,0,8000,
               int(maxMatchDist_/10), 0, maxMatchDist_+10.0 );
       
        h_Particle_Energy_All = new TH2F(
               "Particle_Energy_All",
               ";EcalSummedEnergy;Particle Energy [MeV];Count",
               800,0,8000,
               400,0,4000);

        h_Particle_Kinetic_All = new TH2F(
               "Particle_Kinetic_All",
               ";EcalSummedEnergy;Particle Kinetic Energy [MeV];Count",
               800,0,8000,
               400,0,4000);

        h_Particle_Energy_Matched = new TH2F(
               "Particle_Energy_Matched",
               ";EcalSummedEnergy;Energy of Particles matched to HcalHit [MeV];Count",
               800,0,8000,
               400,0,4000);

        h_Particle_Kinetic_Matched = new TH2F(
               "Particle_Kinetic_Matched",
               ";EcalSummedEnergy;Matched Particle Kinetic Energy [MeV];Count",
               800,0,8000,
               400,0,4000);

        h_HcalHit_Depth_Side = new TH2I(
               "HcalHit_Depth_Side",
               ";EcalSummedEnergy;Depth of Hits in Side HCAL [layer index];Count",
               800,0.,8000.,
               35, 0, 35);

        h_HcalHit_Depth_Back = new TH2I(
               "HcalHit_Depth_Back",
               ";EcalSummedEnergy;Depth of Hits in Back HCAL [layer index];Count",
               800,0,8000,
               100,0,100);

        h_HcalHit_Z_Side = new TH2F(
               "HcalHit_Z_Side",
               ";EcalSummedEnergy;Z Location of Hits in Side HCAL [mm];Count",
               800,0,8000,
               600,0,600);

        h_HcalHit_ZbyR_All = new TH3F(
               "HcalHit_ZbyR_All", 
               "All Hcal Hit Locations;EcalSummedEnergy;Z depth [mm];radial distance from z-axis [mm]",
               800,0,8000,
               500,0,5000,
               220,0,2200);

        h_HcalHit_ZbyR_Unmatched = new TH3F(
                "HcalHit_ZbyR_Unmatched",
               "Hcal unmatched hit locations;EcalSummedEnergy;Z depth [mm];radial distance from z-axis [mm]",
               800,0,8000,
               500,0,5000,
               220,0,2200);

        h_HcalHit_ZbyR_Matched_Photon = new TH3F(
               "HcalHit_ZbyR_Matched_Photon", 
               "Hcal photon hit locations;EcalSummedEnergy;Z depth [mm];radial distance from z-axis [mm]",
               800,0,8000,
               100,0,5000,
               44,0,2200);

        h_HcalHit_ZbyR_Matched_Electron = new TH3F(
               "HcalHit_ZbyR_Matched_Electron", 
               "Hcal electron hit locations;EcalSummedEnergy;Z depth [mm];radial distance from z-axis [mm]",
               800,0,8000,
               100,0,5000,
               44,0,2200);

        h_HcalHit_ZbyR_Matched_Neutron = new TH3F(
               "HcalHit_ZbyR_Matched_Neutron", 
               "Hcal neutron hit locations;EcalSummedEnergy;Z depth [mm];radial distance from z-axis [mm]",
               800,0,8000,
               100,0,5000,
               44,0,2200);

        h_HcalHit_ZbyR_Matched_Other = new TH3F(
               "HcalHit_ZbyR_Matched_Other", 
               "Hcal other particle hit locations;EcalSummedEnergy;Z depth [mm];radial distance from z-axis [mm]",
               800,0,8000,
               100,0,5000,
               44,0,2200);

        h_HcalHit_PE_All = new TH2F(
               "HcalHit_PE_All",
               ";EcalSummedEnergy;PEs of all HcalHits;Count",
               800,0,8000,
               500,0,500);

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
        

        printf( "\n" );
        printf( "===================================\n" );
        printf( "=           HcalHitMatcher        =\n" );
        printf( "===================================\n" );
        printf( "Number of Events         : %8i\n" , numEvents_ );
        printf( "Number of Non Noise Hits : %8i\n" , numNonNoiseHits_ );
        printf( "Number of Matched Hits   : %8i\n" , numMatchedHits_ );
        printf( "Hit Rate (hits/events)   : %8.6f\n" , hitRate );
        printf( "Match Rate (matches/hits): %8.6f\n" , matchRate );
        printf( "===================================\n" );
        printf( "      PDG ID |  Number : Event Rate\n" );
        for ( std::pair<const int,long int> &pdg_N : numParticles_ ) {
            printf( " %11i | %7ld : %10.6f\n" , pdg_N.first , pdg_N.second , ((double)pdg_N.second)/numEvents_ );
        }
        printf( "===================================\n" );

        return;
    }

    double HcalHitMatcher::pointRayDistance(TVector3 rayOrigin, TVector3 rayDirection, TVector3 point)  {
    
        TVector3 originToPoint( point.x() - rayOrigin.x() , point.y() - rayOrigin.y() , point.z() - rayOrigin.z() );

        //Define a parameter t >= 0 that parameterizes the ray and whose value specifies
        //the point on the ray closes to point
        double t = std::max( 0.0 , rayDirection.Dot( originToPoint ) / rayDirection.Mag2() );
    
        //Define vector from ray to point
        TVector3 distVec( 
                        point.x() - (rayOrigin.x() + t*rayDirection.x()) ,
                        point.y() - (rayOrigin.y() + t*rayDirection.y()) ,
                        point.z() - (rayOrigin.z() + t*rayDirection.z()) 
                    );
    
        return distVec.Mag();
    }

} //ldmx namespace

DECLARE_ANALYZER_NS(ldmx, HcalHitMatcher);
