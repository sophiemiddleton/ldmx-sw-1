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

namespace ldmx { 

    DarkBremFilter::DarkBremFilter(const std::string& name, Parameters& parameters)
        : UserAction(name, parameters) {

        std::string desiredVolume = parameters.getParameter< std::string >("volume");
        verbosity_ = parameters.getParameter< int >("verbosity");
        nGensFromPrimary_ = parameters.getParameter< int >("nGensFromPrimary");

        //re-set parameters to reasonable defaults
        //  getParameter returns compiler minimum if parameter is not provided
        if ( verbosity_ < 0 ) verbosity_ = 0;
        if ( desiredVolume.empty() ) desiredVolume = "target";
        if ( nGensFromPrimary_ < 0 ) nGensFromPrimary_ = 0;
 
        //TODO check if this needs to be updated when v12 geo updates are merged in
        for (G4LogicalVolume* volume : *G4LogicalVolumeStore::GetInstance()) {
            G4String volumeName = volume->GetName();
            if ((desiredVolume.compare("ecal") == 0) 
                    and (volumeName.contains("Si") or volumeName.contains("W")) 
                    and volumeName.contains("volume")) {
                volumes_.push_back( volume );
            } else if (volumeName.contains(desiredVolume)) {
                volumes_.push_back( volume );
            }
        }

        std::cout << "DarkBremFilter: "
            << "Looking for A' in: ";
        for ( auto const& volume : volumes_ ) std::cout << volume->GetName() << ", ";
        std::cout << std::endl;
    }

    void DarkBremFilter::BeginOfEventAction(const G4Event*) {
        std::cout << "DarkBremFilter: "
                  << "Beginning event, resetting currentGen and foundAp" 
                  << std::endl;
        currentGen_ = 0;
        foundAp_    = false;
        return;
    }

    G4ClassificationOfNewTrack DarkBremFilter::ClassifyNewTrack(const G4Track* aTrack, const G4ClassificationOfNewTrack& ) {

        if ( aTrack->GetParticleDefinition() == G4APrime::APrime() ) {
            //there is an A'! Yay!
            //  we need to check that it originated in the desired volume
            //  keep A' in the current generation so that we can have it be processed
            //  before the abort event check
            std::cout << "DarkBremFilter: "
                      << "Found A', still need to check if it originated in requested volume." 
                      << std::endl;
            foundAp_ = true;
            return fUrgent; 
        }

        return fWaiting;
    }

    void DarkBremFilter::NewStage() {

        /**
         * called when urgent stack is empty 
         * since we are putting everything on waiting stack, 
         * this is only called when a generation has been simulated 
         * and we are starting the next one.
         */

        std::cout << "DarkBremFilter: "
            << "Closing up generation " << currentGen_ << " and starting a new one."
            << std::endl;

        //increment current generation
        currentGen_++;

        if ( currentGen_ > nGensFromPrimary_ ) {
            //we finished the number of generations that are allowed to produce A'
            //  check if A' was produced
            if ( not foundAp_ ) {
                //A' wasn't produced, abort event
                std::cout << "DarkBremFilter: "
                    << "A' wasn't produced, aborting event." 
                    << std::endl;
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

            //make sure A' is persisted into output file
            UserTrackInformation* userInfo 
              = dynamic_cast<UserTrackInformation*>(track->GetUserInformation());
            userInfo->setSaveFlag(true); 

            //check if A' was made in the desired volume
            if ( not inDesiredVolume(track) ) {
                //abort event if A' wasn't in correct volume
                std::cout << "DarkBremFilter: "
                    << "A' wasn't produced inside of requested volume, aborting event." 
                    << std::endl;
                G4RunManager::GetRunManager()->AbortEvent();
            } else {
                std::cout << "DarkBremFilter: "
                    << "A' was produced inside of the requested volume. Yay!" 
                    << std::endl;
            }

        }//track is A'

        return;
    }

    bool DarkBremFilter::inDesiredVolume(const G4Track* track) const {

        /**
         * Comparing the pointers to logical volumes isn't very robust.
         * TODO find a better way to do this
         */

        auto inVol = track->GetLogicalVolumeAtVertex();
        for ( auto const& volume : volumes_ ) {
            if ( inVol == volume ) return true;
        }

        return false;
    }
}

DECLARE_ACTION(ldmx, DarkBremFilter) 
