
#include "SimApplication/GDMLParser.h"

/**************/
/*   Geant4   */
/**************/
#include "G4VPhysicalVolume.hh" 

/************/
/*   LDMX   */
/************/
#include "SimApplication/AuxInfoReader.h" 

namespace ldmx { 

    GDMLParser::GDMLParser() {
        
        messenger_ = new G4GDMLMessenger(parser_);
        auxInfoReader_ = new AuxInfoReader(parser_);  
    }
    
    GDMLParser::~GDMLParser() {
        delete messenger_; 
        delete parser_;
        delete auxInfoReader_;  
    }
            
    G4VPhysicalVolume* GDMLParser::GetWorldVolume() { 
        auxInfoReader_->readGlobalAuxInfo(); 
        auxInfoReader_->assignAuxInfoToVolumes(); 
        return parser_->GetWorldVolume(); 
    }

}  // ldmx
