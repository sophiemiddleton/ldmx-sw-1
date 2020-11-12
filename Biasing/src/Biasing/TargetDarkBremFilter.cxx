/*
 * @file TargetDarkBremFilter.cxx
 * @class TargetDarkBremFilter
 * @brief Class defining a UserActionPlugin that allows a user to filter out 
 *        events that don't result in a dark brem within a given volume
 * @author Michael Revering, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "Biasing/TargetDarkBremFilter.h"

#include "SimCore/G4APrime.h" //checking if particles match A'
#include "SimCore/UserTrackInformation.h" //make sure A' is saved
#include "SimCore/UserEventInformation.h" //set the weight for the event

#include "G4LogicalVolumeStore.hh" //for the store
#include "G4Electron.hh" //to check if track is electron

namespace ldmx { 

    TargetDarkBremFilter::TargetDarkBremFilter(const std::string& name, Parameters& parameters)
        : UserAction(name, parameters) { threshold_ = parameters.getParameter< double >("threshold"); }

    void TargetDarkBremFilter::stepping(const G4Step* step) { 

        //don't process if event is already aborted
        if (G4EventManager::GetEventManager()->GetConstCurrentEvent()->IsAborted()) return;

        // Get the track associated with this step.
        auto track{step->GetTrack()};

        // Leave if track is not primary
        if (track->GetParentID() != 0) return;

        // Leave if track is not an electron
        if (track->GetParticleDefinition() != G4Electron::Electron()) return; 

        // Leave if track is not in target region
        if (not isInTargetRegion(track->GetVolume())) return;

        if (
            not isInTargetRegion(track->GetNextVolume()) //leaving target region
            or track->GetTrackStatus() == fStopAndKill //stopping within target region
            or track->GetKineticEnergy() == 0. //stopping within target region
           ) {
            // Get the electron secondries
            const std::vector<G4Track*>* secondaries{step->GetSecondary()};
            if (not secondaries or secondaries->size() == 0) {
                AbortEvent("Primary electron did not create secondaries.");
                return;
            } else {
                //check secondaries to see if we made a dark brem
                for (auto& secondary_track : *secondaries) {
                    if (secondary_track->GetParticleDefinition() == G4APrime::APrime()) {
                        //we found an A', woo-hoo!
    
                        if (secondary_track->GetTotalEnergy() < threshold_) {
                            AbortEvent("A' was not created with total energy above input threshold.");
                            return;
                        }
    
                        if (not isInTargetRegion(secondary_track->GetVolume())) {
                            AbortEvent("A' was not created within target region.");
                            return;
                        }
                       
                        auto event{G4EventManager::GetEventManager()}; 
                        if (event->GetUserInformation() == nullptr) { 
                            event->SetUserInformation(new UserEventInformation()); 
                        }
                        //persist weight
                        dynamic_cast<UserEventInformation*>(event->GetUserInformation())->setWeight(secondary_track->GetWeight());
    
                        //we found a good A', so we can leave
                        return;
                    }//this secondary is an A' 
                }//loop through secondaries
            }//are there secondaries to loop through
    
            //got here without finding A'
            AbortEvent("Primary electron did not create A'.");
            return;
        } //should check secondaries of primary

        //get here if we are the primary in the target region,
        //  but shouldn't check the secondaries yet
        return;
    }

    void TargetDarkBremFilter::AbortEvent(const std::string& reason) const {
        //if ( G4RunManager::GetRunManager()->GetVerboseLevel() > 1 ) {
            std::cout << "[ TargetDarkBremFilter ]: "
                << "(" << G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID() << ") "
                << reason << " Aborting event."
                << std::endl;
        //}
        G4RunManager::GetRunManager()->AbortEvent();
        return;
    }
}

DECLARE_ACTION(ldmx, TargetDarkBremFilter) 