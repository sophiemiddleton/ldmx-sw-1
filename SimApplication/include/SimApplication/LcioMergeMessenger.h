#ifndef SIM_APPLICATION_LCIOMERGEMESSENGER_H_
#define SIM_APPLICATION_LCIOMERGEMESSENGER_H_

#include "G4UImessenger.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithABool.hh"

namespace ldmx {

class LcioMergeTool;

class LcioMergeMessenger : public G4UImessenger {

    public:

        LcioMergeMessenger(LcioMergeTool* merge);

        void SetNewValue(G4UIcommand* command, G4String newValues);

    private:

        LcioMergeTool* merge_;

        G4UIdirectory* mergeDir_;
        G4UIdirectory* filterDir_;

        G4UIcmdWithAString* fileCmd_;
        G4UIcmdWithABool* combineCalHitsCmd_;

        G4UIcmdWithADoubleAndUnit* ecalEnergyFilterCmd_;
        G4UIcmdWithAnInteger* eventModulusFilterCmd_;
};

}

#endif
