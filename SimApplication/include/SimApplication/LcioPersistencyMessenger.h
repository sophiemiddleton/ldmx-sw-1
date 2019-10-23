#ifndef SIM_APPLICATION_LCIO_PERSISTENCY_MESSENGER_H_
#define SIM_APPLICATION_LCIO_PERSISTENCY_MESSENGER_H_

#include "G4UImessenger.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithABool.hh"

namespace ldmx {

class LcioPersistencyManager;

// TODO: additional macro commands
// -drop collections
// -enable/disable persistency engine
// -set coll flags
// -configure cal hit contribs
class LcioPersistencyMessenger : public G4UImessenger {

    public:

        LcioPersistencyMessenger(LcioPersistencyManager* mgr);

        void SetNewValue(G4UIcommand* command, G4String newValues);

    private:

        /* The associated persistency manager. */
        LcioPersistencyManager* mgr_{nullptr};

        /* Primary dir for LCIO commands. */
        G4UIdirectory* dir_;

        /* Command to set the output file path. */
        G4UIcmdWithAString* fileCmd_;

        /* Command to set the verbosity. */
        G4UIcmdWithAnInteger* verboseCmd_;

        /*
         * Write mode commands.
         */
        G4UIcommand* newCmd_;
        G4UIcommand* appendCmd_;
        G4UIcommand* recreateCmd_;

        /*
         * Merge tool commands.
         */
        G4UIdirectory* mergeDir_;
        G4UIcmdWithAString* mergeAddCmd_;

        /*
         * Dump event during processing.
         */
        G4UIcmdWithABool* dumpEventDetailedCmd_;
        G4UIcmdWithABool* dumpEventSummaryCmd_;

        /** Dump file. */
        G4UIcommand* dumpFileCmd_;
};

}

#endif
