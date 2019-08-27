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

        ecalXYWidth_ = ps.getDouble( "ecalXYWidth" );
        ecalFrontZ_  = ps.getDouble( "ecalFrontZ" );
        ecalDepth_   = ps.getDouble( "ecalDepth" );

        return;
    }

    void AnalyzePN::analyze(const ldmx::Event& event) {

        const TClonesArray *allSimParticles = event.getCollection( simParticlesCollName_ , simParticlesPassName_ );

        int nSimParticles = allSimParticles->GetEntriesFast();
        for ( int iSP = 0; iSP < nSimParticles; iSP++ ) {
            SimParticle *simParticle = static_cast<SimParticle *>(allSimParticles->At( iSP ));

            std::cout << "Parent: ";
            simParticle->Print();

            if ( simParticle->getProcessType() == SimParticle::ProcessType::photonNuclear ) {
                numPN_++;
                std::cout << "this processType PN" << std::endl;
            }

            std::vector<double> startPoint = simParticle->getVertex();
            if ( isInEcal( startPoint ) ) {
                numStartInEcal_++;
            }

            std::cout << "Children:\n" << std::flush;
            int nChildren = simParticle->getDaughterCount();
            bool foundPNChild = false;
            for ( int iChild = 0; iChild < nChildren; iChild++ ) {
                SimParticle *child = simParticle->getDaughter( iChild );
                if ( !child ) continue;

                child->Print();

                if ( child->getProcessType() == SimParticle::ProcessType::photonNuclear ) {
                    if ( !foundPNChild ) {
                        foundPNChild = true;
                        numChildPN_++;
                        std::cout << "child processType PN" << std::endl;
                    }
                } else if ( foundPNChild ) {
                   //child has processType NOT equal to PN AND there is another child from PN
                   numChildnoPN_++;
                } 
            }
        }

        return;
    }

    void AnalyzePN::onProcessStart() {

        numStartInEcal_ = 0;
        numChildPN_ = 0;
        numChildnoPN_ = 0;
        numPN_ = 0;

        return;
    }

    void AnalyzePN::onProcessEnd() {

        printf( " N Starting in ECAL      : %d\n" , numStartInEcal_ );
        printf( " N Child is PN           : %d\n" , numChildPN_ );
        printf( " N One Child PN, One Not : %d\n" , numChildnoPN_ );
        printf( " N from PN               : %d\n" , numPN_ );

        return;
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

            if ( child and child->getProcessType() == SimParticle::ProcessType::photonNuclear ) {
                return true;
            }
        }

        return false;
    }
}

DECLARE_ANALYZER_NS(ldmx, AnalyzePN);
