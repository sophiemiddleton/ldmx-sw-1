
#ifndef _SIM_APPLICATION_GEO_PARSER_H_
#define _SIM_APPLICATION_GEO_PARSER_H_

/*    Geant4    */
#include "G4VPhysicalVolume.hh"

namespace ldmx { 

    /**
     * Interface for parser used to load geometry into Geant4. 
     */
    class GeoParser {
       
        public:  
        
            /** Destructor */
            virtual ~GeoParser() {}

            /** 
             * Pure virtual method used to retrieve the world volume after parsing.
             *
             * @return The world volume 
             */
            virtual G4VPhysicalVolume* GetWorldVolume() = 0; 
    };
}

#endif // _SIM_APPLICATION_GEO_PARSER_H_
