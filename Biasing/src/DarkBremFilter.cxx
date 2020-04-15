/*
 * @file DarkBremFilter.cxx
 * @class DarkBremFilter
 * @brief Class defining a UserActionPlugin that allows a user to filter out 
 *        events that don't result in a dark brem within a given volume
 * @author Michael Revering, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "Biasing/DarkBremFilter.h"

#include "SimCore/G4APrime.h" //checking if particles match A'
#include "SimCore/UserTrackInformation.h" //make sure A' is saved

#include "G4LogicalVolumeStore.hh" //for the store
#include "G4LogicalVolume.hh" //for IsAncestor

namespace ldmx { 

    DarkBremFilter::DarkBremFilter(const std::string& name, Parameters& parameters)
        : UserAction(name, parameters) {

        std::string volumeName = parameters.getParameter< std::string >("volume");
        verbosity_ = parameters.getParameter< int         >("verbosity");
        nGensFromPrimary_ = parameters.getParameter<int    >("nGensFromPrimary");

        //re-set verbosity and volumes to reasonable defaults
        if ( verbosity_ < 0 ) verbosity_ = 0;
        if ( volumeName.empty() ) volumeName = "target";
        if ( nGensFromPrimary_ < 0 ) nGensFromPrimary_ = 0;

        //Name of ECal mother volume: 'em_calorimeters'
        //Name of Target mother volume: 'target'
        if ( volumeName.compare("ecal") == 0 ) volumeName = "em_calorimeters";
 
        volume_ = G4LogicalVolumeStore::GetInstance()->GetVolume( volumeName.c_str() );

        if ( not volume_ ) {
            EXCEPTION_RAISE(
                    "G4Volume",
                    "Unable to find '" + volumeName + "' in G4LogicalVolumeStore."
                    );
        }
    }

    void DarkBremFilter::BeginOfEventAction(const G4Event*) {
        currentGen_ = -1;
        foundAp_    = false;
        return;
    }

    G4ClassificationOfNewTrack DarkBremFilter::ClassifyNewTrack(const G4Track* aTrack, const G4ClassificationOfNewTrack& ) {

        if ( aTrack->GetParticleDefinition() == G4APrime::APrime() ) {
            //there is an A'! Yay!
            //  we need to check that it originated in the desired volume
            std::cout << "DarkBremFilter: Classifying an A'" << std::endl;
            if ( inDesiredVolume(aTrack->GetLogicalVolumeAtVertex()) ) foundAp_ = true;
        }

        return fWaiting;
    }

    void DarkBremFilter::NewStage() {

        std::cout << "DarkBremFilter::NewStage: " << currentGen_ << std::endl;

        //increment current generation
        currentGen_++;

        if ( currentGen_ > nGensFromPrimary_ ) {
            //we are after the generation after the limit
            //  check if A' was produced
            std::cout << "DarkBremFilter : Past generation limit" << std::endl;
            if ( not foundAp_ ) {
                //A' wasn't produced, abort event
                std::cout << "DarkBremFilter : A' wasn't produced, aborting event." << std::endl;
                G4RunManager::GetRunManager()->AbortEvent();
            }
        }
        
        return;
    }

    void DarkBremFilter::PostUserTrackingAction(const G4Track* track) {

        /* Check that generational stacking is working
        std::cout << "DarkBremFilter:"
            << track->GetTrackID() << " " << track->GetParticleDefinition()->GetPDGEncoding()
            << std::endl;
        */
        
        if ( track->GetParticleDefinition() == G4APrime::APrime() ) {
            //make sure found A' flag is set
            foundAp_ = true;

            //make sure A' is persisted into output file
            UserTrackInformation* userInfo 
              = dynamic_cast<UserTrackInformation*>(track->GetUserInformation());
            userInfo->setSaveFlag(true); 
        }//track is A'

        return;
    }

    bool DarkBremFilter::inDesiredVolume(const G4LogicalVolume* vol) const {

        return true;

        /* TODO check if logical volume at vertex is in desired volume
        //check if vol is nullptr
        if ( vol ) std::cout << vol->GetName() << std::endl;
        else return false; //TODO warning? exception?

        //check if vol is the same as mother volume or if it is an
        //ancestor of the mother volume
        //IsAncestor is the computationally expensive function
        //  but good news, only called maximum once per event
        return (vol->GetName() == volume_->GetName() or volume_->IsAncestor( vol ));
        */
    }
}

DECLARE_ACTION(ldmx, DarkBremFilter) 
