/**
 * @file AnalyzePN.cxx
 * @brief Energy histograms to analyze how PN interactions affect showers in ECAL
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "Ecal/AnalyzePN.h"

namespace ldmx {

    void AnalyzePN::configure(const ldmx::ParameterSet& ps) {

        simParticlesCollName_ = ps.getString( "simParticlesCollName" , "SimParticles" );
        simParticlesPassName_ = ps.getString( "simParticlesPassName" , "sim" );

        ecalDigiCollName_ = ps.getString( "ecalDigiCollName" , "ecalDigis" );
        ecalDigiPassName_ = ps.getString( "ecalDigiPassName" , "" );

        taggerSimHitsCollName_ = ps.getString( "taggerSimHitsCollName" , "TaggerSimHits" );
        taggerSimHitsPassName_ = ps.getString( "taggerSimHitsPassName" , "sim" );

        minPrimaryPhotonEnergy_ = ps.getDouble( "minPrimaryPhotonEnergy" , 2800.0 );
        upstreamLossThresh_ = ps.getDouble( "upstreamLossThresh" , 0.95 );

        //constants to determine if event is saved
        lowReconEnergy_ = ps.getDouble( "lowReconEnergy" , 2000.0 );
        lowPNEnergy_    = ps.getDouble( "lowPNEnergy" , 100.0 );

        return;
    }

    void AnalyzePN::analyze(const ldmx::Event& event) {


        const TClonesArray *taggerSimHits = event.getCollection( taggerSimHitsCollName_ , taggerSimHitsPassName_ );
        if ( checkTagger( taggerSimHits ) ) {
            //something funky happened upstream
            // SKIP EVENT
            skippedEvents_++;
            return;
        }

        const TClonesArray *ecalDigiHits = event.getCollection( ecalDigiCollName_ , ecalDigiPassName_ );
        double ecalReconEnergy = calculateReconEnergy( ecalDigiHits );

        const TClonesArray *allSimParticles = event.getCollection( simParticlesCollName_ , simParticlesPassName_ );

        int nSimParticles = allSimParticles->GetEntriesFast();
        double energyHardestPN = -5.0; //start with negative so if there are no PNs, it goes in the pure EM bin
        double totalEnergyPN = 0.0;
        SimParticle *primaryPhoton = nullptr; //photon with highest energy
        for ( int iSP = 0; iSP < nSimParticles; iSP++ ) {
            SimParticle *simParticle = static_cast<SimParticle *>(allSimParticles->At( iSP ));
            
            if ( !simParticle ) {
                std::cerr << "OOPS! Loaded a nullptr as the sim particle!" << std::endl;
                continue;
            } 

            double energy = simParticle->getEnergy();

            if ( simParticle->getPdgID() == 22 ) {
                if ( (!primaryPhoton and energy > minPrimaryPhotonEnergy_ ) or
                     (primaryPhoton and energy > primaryPhoton->getEnergy())
                    ) {
                    primaryPhoton = simParticle;
                }
            }

            std::vector<double> startPoint = simParticle->getVertex();
            if ( goesPN( simParticle ) ) {

                totalEnergyPN += energy;

                if ( energy > energyHardestPN ) { energyHardestPN = energy; }

            } //particle goes PN and is inside ECAL region

        } //loop through all sim particles

        if ( energyHardestPN < 0.0 ) {
            //no PNs this event
            h_ReconE_NoPN->Fill( ecalReconEnergy );
            energyHardestPN = 0.0; //reset for the histograms with all events in them
        } else if ( primaryPhoton and goesPN( primaryPhoton ) ) {
            //primary photon went PN
            h_ReconE_PrimPhoton->Fill( ecalReconEnergy );
        } else {
            //nothing interesting - primary photon did not go PN but something did go PN
            h_ReconE_HardestPN_NotSpecial->Fill( ecalReconEnergy , energyHardestPN );
            h_ReconE_TotalPN_NotSpecial  ->Fill( ecalReconEnergy , totalEnergyPN );
        }

        h_ReconE_HardestPN_All->Fill( ecalReconEnergy , energyHardestPN );
        h_ReconE_TotalPN_All  ->Fill( ecalReconEnergy , totalEnergyPN );

        if ( totalEnergyPN < lowPNEnergy_ and ecalReconEnergy < lowReconEnergy_ ) {
            //signal region low pn shower - worrisome
            setStorageHint( hint_shouldKeep );
            lowReconLowPN_++;
        }

        return;
    }

    void AnalyzePN::onProcessStart() {

        lowReconLowPN_ = 0;
        skippedEvents_ = 0;
        skippedBecausePrimaryLostEnergy_ = 0;

        TDirectory* baseDirectory = getHistoDirectory();

        h_ReconE_HardestPN_All = new TH2F(
                "ReconE_HardestPN_All",
                ";Reconstructed Energy in ECAL [MeV];Energy of Hardest Photon Going PN [MeV]",
                800,0,8000,
                400,0,4000);

        h_ReconE_TotalPN_All = new TH2F(
                "ReconE_TotalPN_All",
                ";Reconstructed Energy in ECAL [MeV];Total Energy of Photons Going PN [MeV]",
                800,0,8000,
                400,0,4000);

        h_ReconE_HardestPN_NotSpecial = new TH2F(
                "ReconE_HardestPN_NotSpecial",
                "Excluding NoPN and PrimaryPhoton Events;Reconstructed Energy in ECAL [MeV];Energy of Hardest Photon Going PN [MeV]",
                800,0,8000,
                400,0,4000);

        h_ReconE_TotalPN_NotSpecial = new TH2F(
                "ReconE_TotalPN_NotSpecial",
                "Excluding NoPN and PrimaryPhoton Events;Reconstructed Energy in ECAL [MeV];Total Energy of Photons Going PN [MeV]",
                800,0,8000,
                400,0,4000);

        h_ReconE_NoPN = new TH1F(
                "ReconE_NoPN",
                "Only Events without any PN interactions;Reconstructed Energy in ECAL [MeV]",
                800,0,8000);

        h_ReconE_PrimPhoton = new TH1F( 
                "ReconE_PrimPhoton",
                "Only Events with primary photon going PN;Reconstructed Energy in ECAL [MeV]",
                800,0,8000);

        return;
    }

    void AnalyzePN::onProcessEnd() {

        printf( "================================================\n" );
        printf( "| Mid-Shower PN Analyzer                       |\n" );
        printf( "|----------------------------------------------|\n" );
        printf( "| Low PN Events with Recon E < 2.0GeV : %6d |\n" , lowReconLowPN_ );
        printf( "| N Events Skipped for Upstream Loss :  %6d |\n" , skippedEvents_ );
        printf( "| N Events Skipped becasue Primary   :  %6d |\n" , skippedBecausePrimaryLostEnergy_ );
        printf( "================================================\n" );

        return;
    }

    bool AnalyzePN::checkTagger( const TClonesArray *taggerSimHits ) {

        int nTaggerHits = taggerSimHits->GetEntriesFast();
        std::map< int , SimParticle * > upstreamLosses;
        for ( int iHit = 0; iHit < nTaggerHits; iHit++ ) {
            
            SimTrackerHit *taggerHit = (SimTrackerHit *)(taggerSimHits->At( iHit ));
            int trackID = taggerHit->getTrackID();
            SimParticle *particle = taggerHit->getSimParticle();
            std::vector<double> momentum = taggerHit->getMomentum();
            double momentum2 = momentum.at(0)*momentum.at(0) + momentum.at(1)*momentum.at(1) + momentum.at(2)*momentum.at(2);

            if ( particle ) {
                //particle blamed for this hit exists
                double energy = sqrt( momentum2 + particle->getMass()*particle->getMass() );
                
                //Particles except primary are allowed but only if total energy is less than loss threshold
                if ( trackID == 1 and energy < upstreamLossThresh_*4000.0 ) { 
                    //primary lost a lot of energy through non-secondary producing interactions
                    //SKIP EVENT 
                    skippedBecausePrimaryLostEnergy_++;
                    return true;
                } else if ( trackID > 1 and particle->getPdgID() == 11 and energy > upstreamLossThresh_*4000.0 ) {
                    //particle is non-primary electron that could inherit primary electron status
                    //KEEP EVENT
                    return false;
                } else if ( trackID > 1 ) {
                    //particle is NOT primary electron ==> lost energy
                    upstreamLosses[trackID] = particle;
                }

            } else {
                std::cout << "Warning: Tagger Hits's blamed particle was not saved!" << std::endl;
            }
          
        } //loop through tagger hits

        double totalLostEnergy = 0.0;
        for ( std::pair< int , SimParticle* > const& loss : upstreamLosses ) {
            totalLostEnergy += loss.second->getEnergy();
        }
        //std::cout << totalLostEnergy << std::endl;
        //skip event if lost energy is greater than tolerated threshhold
        return (totalLostEnergy > 200.0 ); //(1.0 - upstreamLossThresh_)*4000.0 );
    }

    double AnalyzePN::calculateReconEnergy( const TClonesArray *ecalHitColl ) const {

        double ecalTotalEnergy = 0;
        for(int i=0; i < ecalHitColl->GetEntriesFast(); i++) {
            EcalHit* ecalhit = (EcalHit*)(ecalHitColl->At(i));
            if ( ! ecalhit->isNoise() ) { //Only add non-noise hits
                ecalTotalEnergy += ecalhit->getEnergy();
            }
        }
        
        return ecalTotalEnergy;
    }

    bool AnalyzePN::goesPN( const SimParticle *particle ) const {
        
        int nChildren = particle->getDaughterCount();
        for ( int iChild = 0; iChild < nChildren; iChild++ ) {
            SimParticle *child = particle->getDaughter( iChild );

            //need to check if child pointer is not NULL
            //  pointer to child is a TRef so it will be NULL unless the child was also saved and loaded in
            if ( child and child->getProcessType() == SimParticle::ProcessType::photonNuclear ) {
                return true;
            }
        }

        return false;
    }

}

DECLARE_ANALYZER_NS(ldmx, AnalyzePN);
