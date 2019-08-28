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

        ecalXYWidth_ = ps.getDouble( "ecalXYWidth" );
        ecalFrontZ_  = ps.getDouble( "ecalFrontZ" );
        ecalDepth_   = ps.getDouble( "ecalDepth" );

        return;
    }

    void AnalyzePN::analyze(const ldmx::Event& event) {

        const TClonesArray *ecalDigiHits = event.getCollection( ecalDigiCollName_ , ecalDigiPassName_ );
        double ecalReconEnergy = calculateReconEnergy( ecalDigiHits );

        const TClonesArray *allSimParticles = event.getCollection( simParticlesCollName_ , simParticlesPassName_ );

        int nSimParticles = allSimParticles->GetEntriesFast();
        double energyHardestPN = -5.0; //start with negative so if there are no PNs, it goes in the pure EM bin
        double energyPrimaryPhoton = 0.0;
        double totalEnergyPN = 0.0;
        bool primaryWentPN = false;
        int nPrimaryPhotons = 0;
        for ( int iSP = 0; iSP < nSimParticles; iSP++ ) {
            SimParticle *simParticle = static_cast<SimParticle *>(allSimParticles->At( iSP ));

            if ( !simParticle ) {
                std::cout << "OOPS! Loaded a nullptr as the sim particle!" << std::endl;
                continue;
            }

            double energy = simParticle->getEnergy();
            bool isPrimaryPho = isPrimaryPhoton( simParticle );

            if ( isPrimaryPho ) {
                energyPrimaryPhoton = energy;
                nPrimaryPhotons++;
            }

            std::vector<double> startPoint = simParticle->getVertex();
            if ( goesPN( simParticle ) ) {

                totalEnergyPN += energy;

                if ( isPrimaryPho ) { primaryWentPN = true; }

                if ( energy > energyHardestPN ) { energyHardestPN = energy; }

            } //particle goes PN and is inside ECAL region

        } //loop through all sim particles

        if ( energyHardestPN < 0.0 ) {
            //no PNs this event
            h_ReconE_NoPN->Fill( ecalReconEnergy );
            energyHardestPN = 0.0;
        } else if ( primaryWentPN ) {
            //primary photon went PN
            h_ReconE_PrimPhoton->Fill( ecalReconEnergy );
        } else {
            //nothing interesting
            h_ReconE_HardestPN_NotSpecial->Fill( ecalReconEnergy , energyHardestPN );
            h_ReconE_TotalPN_NotSpecial  ->Fill( ecalReconEnergy , totalEnergyPN );
        }

        h_ReconE_HardestPN_All->Fill( ecalReconEnergy , energyHardestPN );
        h_ReconE_TotalPN_All  ->Fill( ecalReconEnergy , totalEnergyPN );

        if ( nPrimaryPhotons != 1 ) {
            numMiscountPrimary_++;
            std::cout << "N Primary Photons: " << nPrimaryPhotons << std::endl;
        }

        return;
    }

    void AnalyzePN::onProcessStart() {

        numMiscountPrimary_ = 0;

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
        printf( "| N Events Missing Primary Photon: %10d |\n" , numMiscountPrimary_ );
        printf( "================================================\n" );

        return;
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

    bool AnalyzePN::isMidShowerPN( const SimParticle *particle ) const {

        std::vector<double> startPoint = particle->getVertex();
        return ( isInEcal( startPoint) and goesPN( particle ) );
    }
        
    bool AnalyzePN::isInEcal( const std::vector<double> &point ) const {
        
        return ( 
                (abs(point.at(0)) < ecalXYWidth_/2) //x axis
                and (abs(point.at(1)) < ecalXYWidth_/2) //y axis
                and (point.at(2) > ecalFrontZ_) //larger than front z
                and (point.at(2) < ecalFrontZ_+ecalDepth_) //smaller than back z
               );
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

    bool AnalyzePN::isPrimaryPhoton( const SimParticle *particle ) const {
        
        if ( particle->getPdgID() != 22 ) { return false; }

        int nParents = particle->getParentCount();
        for ( int iParent = 0; iParent < nParents; iParent++ ) {
            SimParticle *parent = particle->getParent( iParent );

            if ( parent and parent->getTrackID() == 1 ) { return true; }
        }

        return false;
    }
}

DECLARE_ANALYZER_NS(ldmx, AnalyzePN);
