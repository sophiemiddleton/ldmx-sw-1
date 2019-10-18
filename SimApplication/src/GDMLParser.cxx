
#include "SimApplication/GDMLParser.h"

/**************/
/*   Geant4   */
/**************/
#include "G4VPhysicalVolume.hh" 

namespace ldmx { 

    GDMLParser::GDMLParser() {
        
        messenger_ = new G4GDMLMessenger(parser_); 
    }
    
    GDMLParser::~GDMLParser() {
        delete messenger_; 
        delete parser_; 
    }

}  // ldmx
