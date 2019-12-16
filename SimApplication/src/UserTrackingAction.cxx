#include "SimApplication/UserTrackingAction.h"

// LDMX
#include "SimApplication/TrackMap.h"
#include "SimApplication/Trajectory.h"
#include "SimApplication/UserPrimaryParticleInformation.h"
#include "SimApplication/UserRegionInformation.h"
#include "SimCore/UserTrackInformation.h"
#include "SimPlugins/PluginManager.h"

// Geant4
#include "G4PrimaryParticle.hh"
#include "G4TrackingManager.hh"
#include "G4VUserPrimaryParticleInformation.hh"

// STL
#include <iostream>

#include "lcdd/detectors/CurrentTrackState.hh" 

namespace ldmx {

    void UserTrackingAction::PreUserTrackingAction(const G4Track* aTrack) {

        int trackID = aTrack->GetTrackID();
        
        // This is set for LCDD sensitive detectors, which is strange but we don't want to change it right now!
        CurrentTrackState::setCurrentTrackID(trackID);

        if (trackMap_.contains(trackID)) {
            if (trackMap_.hasTrajectory(trackID)) {
                // This makes sure the tracking manager does not delete the trajectory.
                fpTrackingManager->SetStoreTrajectory(true);
            }
        } else {
            // New track so call the process method.
            processTrack(aTrack);
        }

        // Activate user plugins.
        pluginManager_->preTracking(aTrack);
    }

    void UserTrackingAction::PostUserTrackingAction(const G4Track* aTrack) {

        // Activate user plugins.
        pluginManager_->postTracking(aTrack);

        // std::cout << "tracking acition: zpos = " << aTrack->GetPosition().z() << ", pdgid = " << aTrack->GetDefinition()->GetPDGEncoding() << ", volname = " << aTrack->GetVolume()->GetLogicalVolume()->GetName().c_str() << std::endl;
        
        auto info = static_cast<UserTrackInformation*>(aTrack->GetUserInformation());
        // Save extra trajectories on tracks that were flagged for saving during event processing.
        if (info->getSaveFlag()) {
            if (!trackMap_.hasTrajectory(aTrack->GetTrackID())) {
                storeTrajectory(aTrack);
            }
        }

        // Set end point momentum on the trajectory.
        if (fpTrackingManager->GetStoreTrajectory()) {
            auto traj = dynamic_cast<Trajectory*>(fpTrackingManager->GimmeTrajectory());
            if (traj) {
                if (aTrack->GetTrackStatus() == G4TrackStatus::fStopAndKill) {
                    traj->setEndPointMomentum(aTrack);
                }

                traj->setSaveFlag(dynamic_cast<UserTrackInformation*>(aTrack->GetUserInformation())->getSaveFlag()); 
            }
        }
    }

    void UserTrackingAction::storeTrajectory(const G4Track* aTrack) {

        // Create a new trajectory for this track.
        fpTrackingManager->SetStoreTrajectory(true);
        Trajectory* traj = new Trajectory(aTrack);
        fpTrackingManager->SetTrajectory(traj);

        // Update the gen status from the primary particle.
        if (aTrack->GetDynamicParticle()->GetPrimaryParticle() != NULL) {
            G4VUserPrimaryParticleInformation* primaryInfo = aTrack->GetDynamicParticle()->GetPrimaryParticle()->GetUserInformation();
            if (primaryInfo != NULL) {
                traj->setGenStatus(((UserPrimaryParticleInformation*) primaryInfo)->getHepEvtStatus());
            }
        }

        // Map track ID to trajectory.
        trackMap_.addTrajectory(traj);
    }

    void UserTrackingAction::processTrack(const G4Track* aTrack) {

        // Set user track info on new track.
        UserTrackInformation* trackInfo{nullptr}; 
        if (!aTrack->GetUserInformation()) {
            trackInfo = new UserTrackInformation;
            trackInfo->setInitialMomentum(aTrack->GetMomentum());
            const_cast<G4Track*>(aTrack)->SetUserInformation(trackInfo);
        }

        // Check if trajectory storage should be turned on or off from the region info.
        UserRegionInformation* regionInfo = (UserRegionInformation*) aTrack->GetLogicalVolumeAtVertex()->GetRegion()->GetUserInformation();
        bool aboveEnergyThreshold = false;
        bool storeSecondaries = false; 
        if (regionInfo) {
            //regionInfo->Print();
            //std::cout << "threshold: " << regionInfo->getThreshold() << std::endl; 
            aboveEnergyThreshold = (aTrack->GetKineticEnergy() > regionInfo->getThreshold());
            storeSecondaries = regionInfo->getStoreSecondaries(); 
        }

        // Check if trajectory storage should be turned on or off from the gen status info
        int curGenStatus = -1;
        if (aTrack->GetDynamicParticle()->GetPrimaryParticle() != NULL){
            G4VUserPrimaryParticleInformation* primaryInfo = aTrack->GetDynamicParticle()->GetPrimaryParticle()->GetUserInformation();
            curGenStatus = ((UserPrimaryParticleInformation*) primaryInfo)->getHepEvtStatus();
        }

        // Always save a particle if it has gen status == 1
        if (curGenStatus == 1){
            storeTrajectory(aTrack);
            trackInfo->setSaveFlag(true); 
        } else if (storeSecondaries && aboveEnergyThreshold) {
            storeTrajectory(aTrack); 
            trackInfo->setSaveFlag(true); 
        } else {
        //else if (!storeSecondaries) {
            // Turn off trajectory storage for this track from region flag.
            fpTrackingManager->SetStoreTrajectory(false);
            trackInfo->setSaveFlag(false); 
        } /*else if (regionInfo && (regionInfo->getStoreSecondaries() && aboveEnergyThreshold)) { 
            storeTrajectory(aTrack);
            trackInfo->setSaveFlag(true); 
        } */
        //else {
            // Store a new trajectory for this track.
            //storeTrajectory(aTrack);
            //trackInfo->setSaveFlag(true); 
        //}

        // Save the association between track ID and its parent ID for all tracks in the event.
        trackMap_.addSecondary(aTrack->GetTrackID(), aTrack->GetParentID());
    }
}
