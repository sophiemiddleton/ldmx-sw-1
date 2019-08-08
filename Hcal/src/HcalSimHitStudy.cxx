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

        knownPDGs_ = ps.getVInteger( "knownPDGs" );
        
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
            if ( pdgID < 1000000000 ) {
                pdgStr.Form( "%d" , pdgID );
            } else {
                pdgStr.Form( "Nuclei" );
            }

            float energyDep = simHit->getEdep();
            std::vector<float> position = simHit->getPosition();
            double zPos = position.at(2);
            //double radialPos = pow( pow( position.at(0) , 2 ) + pow( position.at(1) , 2 ) , 0.5 );

            // Copying ID unpacking procedure from HcalDigiProducer
            HcalID currID;
            currID.setRawValue( simHit->getID() );
            currID.unpack();
            int layer = currID.getFieldValue("layer");
            int section = currID.getFieldValue("section");

            if ( section == 0 ) {
                h_Back_Depth->Fill( pdgStr , layer , 1. );
            } else if ( section > 0 and section < 5 ) {
                h_Side_Z->Fill( pdgStr , zPos - ecalFrontZ_ , 1. );
                h_Side_Depth->Fill( pdgStr , layer , 1. );
            } else {
                std::cerr << "[ Warning ] : HcalHitMatcher::analyze - found HcalSection " << section
                    << " that is not in the correct range." << std::endl;
                continue;
            }

            h_EnergyDep->Fill( pdgStr , energyDep , 1. );
            //h_ZbyR->Fill( pdgStr , zPos , radialPos , 1. );

        } //loop through HcalSimHits ==> iHit and simHit

        return;
    }

    void HcalSimHitStudy::onProcessStart() {

        getHistoDirectory();

        h_Side_Depth = new TH2I(
                "h_Side_Depth",
                ";PDG ID;Depth of Hit in Side HCAL [layer index]",
                knownPDGs_.size()+1, 0, knownPDGs_.size()+1,
                35, 0 , 35 );

        h_Back_Depth = new TH2I(
                "h_Back_Depth",
                ";PDG ID;Depth of Hit in Back HCAL [layer index]",
                knownPDGs_.size()+1, 0, knownPDGs_.size()+1,
                100, 0, 100 );

        h_Side_Z = new TH2F(
                "h_Side_Z",
                ";PDG ID;Z Position of Hit in Side HCAL [mm]",
                knownPDGs_.size()+1, 0, knownPDGs_.size()+1,
                120, 0, 600 );

        int nEnergyBins = 37;
        double energyBins[38] = { 0.0 ,
            0.01 , 0.02 , 0.03 , 0.04 , 0.05 , 0.06 , 0.07 , 0.08 , 0.09 , 
            0.1 , 0.2 , 0.3 , 0.4 , 0.5 , 0.6 , 0.7 , 0.8 , 0.9 , 
            1.0 , 2.0 , 3.0 , 4.0 , 5.0 , 6.0 , 7.0 , 8.0, 9.0 , 
            10.0 , 20.0 , 30.0 , 40.0 , 50.0 , 60.0 , 70.0 , 80.0 , 90.0 , 100.0 };
        h_EnergyDep = new TH2F(
                "h_EnergyDep",
                ";PDG ID;Energy Deposited in Hcal [MeV]",
                knownPDGs_.size()+1, 0, knownPDGs_.size()+1,
                nEnergyBins , energyBins );

//        h_ZbyR = new TH3F(
//                "h_ZbyR",
//                "Hcal Sim Hit Locations;PDG ID;Z Postion [mm];Radial Position [mm]",
//                knownPDGs_.size()+1, 0, knownPDGs_.size()+1,
//                500,0,5000,
//                220,0,2200);

        //set pdg bin labels
        for( int ibin = 1; ibin < knownPDGs_.size()+1; ibin++ ) {
            TString pdgStr;
            pdgStr.Form( "%d" , knownPDGs_.at(ibin-1) );
            h_Side_Depth->GetXaxis()->SetBinLabel( ibin , pdgStr );
            h_Back_Depth->GetXaxis()->SetBinLabel( ibin , pdgStr );
            h_Side_Z    ->GetXaxis()->SetBinLabel( ibin , pdgStr );
            h_EnergyDep ->GetXaxis()->SetBinLabel( ibin , pdgStr );
//            h_ZbyR      ->GetXaxis()->SetBinLabel( ibin , pdgStr );
        }

        //last bin is for nuclei
        TString pdgStr( "Nuclei" );
        h_Side_Depth->GetXaxis()->SetBinLabel( knownPDGs_.size()+1 , pdgStr );
        h_Back_Depth->GetXaxis()->SetBinLabel( knownPDGs_.size()+1 , pdgStr );
        h_Side_Z    ->GetXaxis()->SetBinLabel( knownPDGs_.size()+1 , pdgStr );
        h_EnergyDep ->GetXaxis()->SetBinLabel( knownPDGs_.size()+1 , pdgStr );
//        h_ZbyR      ->GetXaxis()->SetBinLabel( knownPDGs_.size()+1 , pdgStr );

        return;
    }

    void HcalSimHitStudy::onProcessEnd() {

        return;
    }

}

DECLARE_ANALYZER_NS(ldmx, HcalSimHitStudy);
