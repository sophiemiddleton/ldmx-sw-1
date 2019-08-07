/**
 * @file HcalSimHitStudy.cxx
 * @brief 
 * @author 
 */

#include "Hcal/HcalSimHitStudy.h"

namespace ldmx {

    void HcalSimHitStudy::configure(const ldmx::ParameterSet& ps) {

        hcalSimHitColl_ = ps.getString( "hcalSimHitColl" , EventConstants::HCAL_SIM_HITS );
        hcalSimHitPass_ = ps.getString( "hcalSimHitPass" , "sim" );

        backZeroLayer_ = ps.getDouble( "backZeroLayer" );
        sideZeroLayer_ = ps.getDouble( "sideZeroLayer" );

        ecalFrontZ_ = ps.getDouble( "ecalFrontZ" );
        
        return;
    }

    void HcalSimHitStudy::analyze(const ldmx::Event& event) {

        const TClonesArray *hcalSimHits = event.getCollection( hcalSimHitColl_ , hcalSimHitPass_ );

        for( int iHit = 0; iHit < hcalSimHits->GetEntriesFast(); iHit++ ) {
            SimCalorimeterHit* simHit = (SimCalorimeterHit *)(hcalSimHits->At( iHit ) );

            if ( simHit->getNumberOfContribs() != 1 ) {
                std::cout << "I didn't think this would happen! There are " << simHit->getNumberOfContribs()
                    << " which isn't equal to 1!" << std::endl;
                continue;
            }

            int pdgID = simHit->getContrib(0).pdgCode;
            TString pdgStr;
            pdgStr.Form( "%d" , pdgID );
            float energyDep = simHit->getEdep();
            std::vector<float> position = simHit->getPosition();
            double zPos = position.at(2);
            double radialPos = pow( pow( position.at(0) , 2 ) + pow( position.at(1) , 2 ) , 0.5 );

            if ( zPos < backZeroLayer_ ) {
                h_Side_Z->Fill( pdgStr , zPos - ecalFrontZ_ );
            } else {
                h_Back_Z->Fill( pdgStr , zPos - backZeroLayer_ );
            }

            h_EnergyDep->Fill( pdgStr , energyDep );
            h_ZbyR->Fill( pdgStr , zPos , radialPos );

            //TODO calculate depth without section information
//            double depth = 0;
//            if ( section == 0 ) {
//                //back - use z
//                depth = hcalhit->getZ() - backZeroLayer_;
//            } else if ( section == 1 or section == 2 ) {
//                //top or bottom - use y
//                depth = abs(hcalhit->getY()) - sideZeroLayer_;
//            } else if ( section == 3 or section == 4 ) {
//                //left or right - use x
//                depth = abs(hcalhit->getX()) - sideZeroLayer_;
//            } else {
//                std::cerr << "[ Warning ] : HcalHitMatcher::analyze - found HcalSection " << section
//                    << " that is not in the correct range." << std::endl;
//            }


        } //loop through HcalSimHits ==> iHit and simHit

        return;
    }

    void HcalSimHitStudy::onProcessStart() {

        //add in bins of known particles
        std::vector<std::string> knownPDGs = { "22" , "11" , "-11" , "13" , "-13", 
            "2112" , "2212" , "211", "-211" , "130", "321" , "1000010020" };

        getHistoDirectory();

        h_Side_Depth = new TH2F(
                "h_Side_Depth",
                ";PDG ID;Depth of Hit in Side HCAL [mm]",
                knownPDGs.size(), 0, knownPDGs.size(),
                120, 0, 600,
                40, 0, 1600 );

        h_Side_Z = new TH2F(
                "h_Side_Z",
                ";PDG ID;Z Position of Hit in Side HCAL [mm]",
                knownPDGs.size(), 0, knownPDGs.size(),
                120, 0, 600 );

        h_Back_Z = new TH2F(
                "h_Back_Z",
                ";PDG ID;Depth of Hit in Back HCAL [mm]",
                knownPDGs.size(), 0, knownPDGs.size(),
                100, 0, 4400 );

        h_EnergyDep = new TH2F(
                "h_EnergyDep",
                ";PDG ID;Energy Deposited in Hcal [MeV]",
                knownPDGs.size(), 0, knownPDGs.size(),
                80, 0, 400 );
        h_EnergyDep->SetCanExtend( kYaxis );

        h_ZbyR = new TH3F(
                "h_ZbyR",
                "Hcal Sim Hit Locations;PDG ID;Z Postion [mm];Radial Position [mm]",
                knownPDGs.size(), 0, knownPDGs.size(),
                500,0,5000,
                220,0,2200);

        //set pdg bin labels
        for( int ibin = 1; ibin < knownPDGs.size()+1; ibin++ ) {
            h_Side_Depth->GetYaxis()->SetBinLabel( ibin , knownPDGs.at(ibin-1).c_str() );
            h_Side_Z    ->GetYaxis()->SetBinLabel( ibin , knownPDGs.at(ibin-1).c_str() );
            h_Back_Z    ->GetYaxis()->SetBinLabel( ibin , knownPDGs.at(ibin-1).c_str() );
            h_EnergyDep ->GetYaxis()->SetBinLabel( ibin , knownPDGs.at(ibin-1).c_str() );
            h_ZbyR      ->GetYaxis()->SetBinLabel( ibin , knownPDGs.at(ibin-1).c_str() );
        }

        return;
    }

    void HcalSimHitStudy::onProcessEnd() {

        return;
    }

}

DECLARE_ANALYZER_NS(ldmx, HcalSimHitStudy);
