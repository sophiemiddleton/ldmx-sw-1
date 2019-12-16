/**
 *
 */

#ifndef _SIM_APPLICATION_LCDD_PARSER_H_
#define _SIM_APPLICATION_LCDD_PARSER_H_

/**************/
/*   Geant4   */
/**************/
#include "G4GDMLParser.hh"

/************/
/*   LDMX   */
/************/
#include "SimApplication/GeoParser.h"

#include "lcdd/core/LCDDMessenger.hh"
#include "lcdd/util/GDMLWriterMessenger.hh"
//#include "lcdd/core/LCDDObjectStoreInspector.hh"

// Forward Declarations
class G4VPhysicalVolume;

namespace ldmx { 

    /**
     */
    class LCDDparser : public GeoParser { 
    
        public: 

            /** Constructor */
            LCDDparser(); 

            /** Destructor */
            ~LCDDparser(); 
            
            G4VPhysicalVolume* GetWorldVolume(); 


        private:

            LCDDMessenger* messenger_{new LCDDMessenger()};

            GDMLWriterMessenger* writer_{new GDMLWriterMessenger()};  

            //LCDDObjectStoreInspector* inspector_{new LCDDObjectStoreInspector()}; 
    
    }; // LCDDparser

} // ldmx

#endif // _SIMAPPLICATION_LCDD_PARSER_H_
