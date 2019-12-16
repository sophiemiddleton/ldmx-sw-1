#ifndef SIM_APPLICATION_LCIO_SIM_PARTICLE_BUILDER_H_
#define SIM_APPLICATION_LCIO_SIM_PARTICLE_BUILDER_H_

//#include "UserTrackingAction.h"

#include "EVENT/LCIO.h"
#include "IMPL/MCParticleImpl.h"
#include "IMPL/LCCollectionVec.h"

#include "G4SystemOfUnits.hh"

#include "SimApplication/TrackMap.h"
#include "SimApplication/Trajectory.h"
#include <map>

namespace ldmx {


class LcioSimParticleBuilder {

    public:

        typedef std::map<G4int, IMPL::MCParticleImpl*> MCParticleMap;

        LcioSimParticleBuilder(TrackMap* trackMap);

        virtual ~LcioSimParticleBuilder();

        void buildParticleMap(G4TrajectoryContainer* trajectories, IMPL::LCCollectionVec* collVec);

        IMPL::MCParticleImpl* findMCParticle(G4int trackID);

        void buildMCParticle(Trajectory* traj);

        IMPL::LCCollectionVec* buildMCParticleColl(const G4Event* anEvent);

        TrackMap& getTrackMap();

    private:

        MCParticleMap particleMap_;

        TrackMap* trackMap_;
};
}



#endif
