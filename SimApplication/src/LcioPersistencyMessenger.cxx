#include "SimApplication/LcioPersistencyMessenger.h"

// include only in cxx file due to circular dep!
#include "SimApplication/LcioPersistencyManager.h"

#include <sstream>

namespace ldmx {

LcioPersistencyMessenger::LcioPersistencyMessenger(LcioPersistencyManager* mgr) : mgr_{mgr} {
    dir_ = new G4UIdirectory("/hps/lcio/", this);
    fileCmd_ = new G4UIcmdWithAString("/hps/lcio/file", this);
    verboseCmd_ = new G4UIcmdWithAnInteger("/hps/lcio/verbose", this);

    newCmd_ = new G4UIcommand("/hps/lcio/new", this);
    newCmd_->SetGuidance("Write a new LCIO file and throw an error if the file exists already.");

    recreateCmd_ = new G4UIcommand("/hps/lcio/recreate", this);
    recreateCmd_->SetGuidance("Recreate the output LCIO file and delete the file if it exists already.");

    appendCmd_ = new G4UIcommand("/hps/lcio/append", this);
    appendCmd_->SetGuidance("Append events to an existing LCIO file.");

    mergeDir_ = new G4UIdirectory("/hps/lcio/merge/", this);

    mergeAddCmd_ = new G4UIcmdWithAString("/hps/lcio/merge/add", this);

    dumpEventDetailedCmd_ = new G4UIcmdWithABool("/hps/lcio/dumpEventDetailed", this);
    dumpEventDetailedCmd_->GetParameter(0)->SetOmittable(true);
    dumpEventDetailedCmd_->GetParameter(0)->SetDefaultValue("true");

    dumpEventSummaryCmd_ = new G4UIcmdWithABool("/hps/lcio/dumpEventSummary", this);
    dumpEventSummaryCmd_->GetParameter(0)->SetOmittable(true);
    dumpEventSummaryCmd_->GetParameter(0)->SetDefaultValue("true");

    dumpFileCmd_ = new G4UIcommand("/hps/lcio/dumpFile", this);
    auto p = new G4UIparameter("file", 's', false);
    dumpFileCmd_->SetParameter(p);
    p = new G4UIparameter("nevents", 'i', true);
    p->SetDefaultValue(-1);
    dumpFileCmd_->SetParameter(p);
    p = new G4UIparameter("skip", 'i', true);
    p->SetDefaultValue(0);
    dumpFileCmd_->SetParameter(p);
}

void LcioPersistencyMessenger::SetNewValue(G4UIcommand* command, G4String newValues) {
    if (command == this->fileCmd_) {
        std::cout << "LcioPersistencyMessenger: Setting output file to '" << newValues << "'" << std::endl;
        mgr_->setOutputFile(newValues);
    } else if (command == this->verboseCmd_) {
        std::cout << "LcioPersistencyMessenger: Setting verbose level to " << newValues << std::endl;
        mgr_->SetVerboseLevel(G4UIcmdWithAnInteger::GetNewIntValue(newValues));
    } else if (command == this->newCmd_) {
        mgr_->setWriteMode(LcioPersistencyManager::NEW);
    } else if (command == this->recreateCmd_) {
        mgr_->setWriteMode(LcioPersistencyManager::RECREATE);
    } else if (command == this->appendCmd_) {
        mgr_->setWriteMode(LcioPersistencyManager::APPEND);
    } else if (command == this->mergeAddCmd_) {
        mgr_->addMerge(new LcioMergeTool(newValues));
    } else if (command == dumpEventDetailedCmd_) {
        mgr_->setDumpEventDetailed(G4UIcmdWithABool::GetNewBoolValue(newValues));
    } else if (command == dumpEventSummaryCmd_) {
        mgr_->setDumpEventSummary(G4UIcmdWithABool::GetNewBoolValue(newValues));
    } else if (command == dumpFileCmd_) {
        std::stringstream ss(newValues);
        std::string fileName;
        int nevents;        
        int nskip;
        ss >> fileName;
        ss >> nevents;
        ss >> nskip;
        LcioPersistencyManager::dumpFile(fileName, nevents, nskip);
    }
}

}
