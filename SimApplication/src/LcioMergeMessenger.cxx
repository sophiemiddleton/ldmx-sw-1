#include "SimApplication/LcioMergeMessenger.h"

#include "SimApplication/LcioMergeTool.h"

namespace ldmx {

LcioMergeMessenger::LcioMergeMessenger(LcioMergeTool* merge) : merge_(merge) {

    G4String mergePath = "/hps/lcio/merge/" + merge->getName() + "/";
    mergeDir_ = new G4UIdirectory(mergePath, this);

    G4String filterPath = mergePath + "filter/";
    filterDir_ = new G4UIdirectory(filterPath, this);

    G4String filePath = mergePath + "file";
    fileCmd_ = new G4UIcmdWithAString(filePath, this);

    G4String eventModulusPath = filterPath + "eventModulus";
    eventModulusFilterCmd_ = new G4UIcmdWithAnInteger(eventModulusPath, this);

    G4String combineCalHitsPath = mergePath + "combineCalHits";
    combineCalHitsCmd_ = new G4UIcmdWithABool(combineCalHitsPath, this);
    combineCalHitsCmd_->SetDefaultValue(true);

    G4String ecalEnergyFilterPath = filterPath + "ecalEnergy";
    ecalEnergyFilterCmd_ = new G4UIcmdWithADoubleAndUnit(ecalEnergyFilterPath, this);
    ecalEnergyFilterCmd_->GetParameter(0)->SetOmittable(false);
    ecalEnergyFilterCmd_->GetParameter(1)->SetOmittable(true);
    ecalEnergyFilterCmd_->GetParameter(1)->SetDefaultValue("GeV");
}

void LcioMergeMessenger::SetNewValue(G4UIcommand* command, G4String newValues) {
    if (command == fileCmd_) {
        merge_->addFile(newValues);
    } else if (command == eventModulusFilterCmd_) {
        auto filter = new LcioMergeTool::EventModulusFilter();
        auto modulus = G4UIcmdWithAnInteger::GetNewIntValue(newValues);
        filter->setModulus(modulus);
        merge_->addFilter(filter);
    } else if (command == combineCalHitsCmd_) {
        merge_->setCombineCalHits(G4UIcmdWithABool::GetNewBoolValue(newValues));
    } else if (command == ecalEnergyFilterCmd_) {
        auto filter = new LcioMergeTool::EcalEnergyFilter();
        filter->setEnergyCut(G4UIcmdWithADoubleAndUnit::GetNewDoubleValue(newValues));
        merge_->addFilter(filter);
    }
}

}
