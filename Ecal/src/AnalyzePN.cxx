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
        for ( int iSP = 0; iSP < nSimParticles; iSP++ ) {
            SimParticle *simParticle = static_cast<SimParticle *>(allSimParticles->At( iSP ));

            if ( !simParticle ) {
                std::cout << "OOPS! Loaded a nullptr as the sim particle!" << std::endl;
                continue;
            }

            double energy = simParticle->getEnergy();
            std::vector<double> startPoint = simParticle->getVertex();
            if ( isMidShowerPN( simParticle ) and energy > energyHardestPN ) {
                energyHardestPN = energy;
            } //particle goes PN and is inside ECAL region

        } //loop through all sim particles

        h_ReconE_HardestPN->Fill( ecalReconEnergy , energyHardestPN );

        if ( energyHardestPN > hardestOverAllEvents_ ) {
            hardestOverAllEvents_ = energyHardestPN;
        }

        return;
    }

    void AnalyzePN::onProcessStart() {

        hardestOverAllEvents_ = 0.0;

        TDirectory* baseDirectory = getHistoDirectory();

        int nHardestPNBins = 17;
        double hardestPNBins[18] = {
            -10.0 , 0.0 , //negative bin for pure em showers
            5.0 , 
            1e1 , 2e1 , 3e1 , 4e1 , 5e1 , 6e1 , 7e1 , 8e1 , 9e1 ,
            1e2 , 2e2 , 3e2 , 4e2 , 5e2 , 1e3 };

        h_ReconE_HardestPN = new TH2F(
                "ReconE_HardestPN",
                ";Reconstructed Energy in ECAL [MeV];Energy of Hardest Photon Going PN [MeV] (Negative Means Pure EM)",
                800,0,8000,
                nHardestPNBins , hardestPNBins );

        return;
    }

    void AnalyzePN::onProcessEnd() {

        printf( "================================================\n" );
        printf( "| Mid-Shower PN Analyzer                       |\n" );
        printf( "|----------------------------------------------|\n" );
        printf( "| Energy of Hardest PN Photon (all events) :   |\n" );
        printf( "|     %6.2f MeV                               |\n" , hardestOverAllEvents_ );
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
}

DECLARE_ANALYZER_NS(ldmx, AnalyzePN);
