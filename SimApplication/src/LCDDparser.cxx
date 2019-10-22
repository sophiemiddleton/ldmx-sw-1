/**
 *
 */

#include "SimApplication/LCDDparser.h" 

#include "lcdd/core/GeometryManager.hh"
#include "lcdd/core/LCDDParser.hh"

namespace ldmx { 

    LCDDparser::LCDDparser() {
      
        GeometryManager::instance();

        LCDDParser::instance();
    }

    LCDDparser::~LCDDparser() {
        delete messenger_; 
        delete writer_; 
    }

    G4VPhysicalVolume* LCDDparser::GetWorldVolume() {
        
        return LCDDParser::instance()->construct();
    }

} // ldmx

