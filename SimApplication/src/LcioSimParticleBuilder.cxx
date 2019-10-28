#include "SimApplication/LcioSimParticleBuilder.h"


namespace ldmx {

LcioSimParticleBuilder::LcioSimParticleBuilder(TrackMap* trackMap) :
        trackMap_(trackMap) {
}

LcioSimParticleBuilder::~LcioSimParticleBuilder() {
}

void LcioSimParticleBuilder::buildParticleMap(G4TrajectoryContainer* trajectories, IMPL::LCCollectionVec* collVec) {
    particleMap_.clear();
    for (auto trajectory : *trajectories->GetVector()) {
        //if (Trajectory::getTrajectory(trajectory)->getSaveFlag()) {
            auto particle = new IMPL::MCParticleImpl;
            collVec->addElement(particle);
            particleMap_[trajectory->GetTrackID()] = particle;
        //}
    }
}

IMPL::MCParticleImpl* LcioSimParticleBuilder::findMCParticle(G4int trackID) {
    G4VTrajectory* traj = trackMap_->findTrajectory(trackID);
    if (traj != nullptr) {
        return particleMap_[traj->GetTrackID()];
    } else {
        return nullptr;
    }
}

void LcioSimParticleBuilder::buildMCParticle(Trajectory* traj) {

    IMPL::MCParticleImpl* p = particleMap_[traj->GetTrackID()];

    if (!p) {
        std::cerr << "LcioSimParticleBuilder: MCParticle not found for track ID " << traj->GetTrackID() << std::endl;
        G4Exception("SimParticleBuilder::buildSimParticle", "", FatalException, "MCParticle not found for Trajectory.");
    }

    p->setGeneratorStatus(traj->getGenStatus());
    p->setPDG(traj->GetPDGEncoding());
    p->setCharge(traj->GetCharge());
    p->setMass(traj->getMass()/GeV);
    //p->setEnergy(traj->getEnergy());
    p->setTime(traj->getGlobalTime());

    double vertexArr[] = { traj->getVertexPosition()[0], traj->getVertexPosition()[1], traj->getVertexPosition()[2] };
    p->setVertex(vertexArr);

    double momentum[] = { traj->GetInitialMomentum()[0] / GeV, traj->GetInitialMomentum()[1] / GeV,
            traj->GetInitialMomentum()[2] / GeV };
    p->setMomentum(momentum);

    //const G4ThreeVector& endpMomentum = traj->getEndPointMomentum();
    //p->setEndPointMomentum(endpMomentum[0], endpMomentum[1], endpMomentum[2]);

    double endp[] = { traj->getEndPoint()[0], traj->getEndPoint()[1], traj->getEndPoint()[2] };
    p->setEndpoint(endp);

    if (traj->GetParentID() > 0) {
        IMPL::MCParticleImpl* parent = findMCParticle(traj->GetParentID());
        if (parent != nullptr) {
            p->addParent(parent);
        }
    }

    // Set sim status to indicate particle was created in simulation.
    if (!traj->getGenStatus()) {
        std::bitset<32> simStatus;
        simStatus[EVENT::MCParticle::BITCreatedInSimulation] = 1;
        p->setSimulatorStatus(simStatus.to_ulong());
    }
}

IMPL::LCCollectionVec* LcioSimParticleBuilder::buildMCParticleColl(const G4Event* anEvent) {

    auto collVec = new IMPL::LCCollectionVec(EVENT::LCIO::MCPARTICLE);
    auto trajectories = anEvent->GetTrajectoryContainer();

    if (trajectories) {

        buildParticleMap(trajectories, collVec);

        for (auto trajectory : *trajectories->GetVector()) {
            //if (Trajectory::getTrajectory(trajectory)->getSaveFlag()) {
                buildMCParticle(static_cast<Trajectory*>(trajectory));
            //}
        }
    }

    return collVec;
}

TrackMap& LcioSimParticleBuilder::getTrackMap() {
    return *trackMap_;
}

} // namespace ldmx
