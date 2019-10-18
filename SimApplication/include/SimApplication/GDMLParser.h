
#ifndef _SIM_APPLICATION_GDML_PARSER_H_
#define _SIM_APPLICATION_GDML_PARSER_H_

/**************/
/*   Geant4   */
/**************/
#include "G4GDMLParser.hh"

/************/
/*   LDMX   */
/************/
#include "SimApplication/GeoParser.h" 

// Forward Declarations
class G4VPhysicalVolume;

namespace ldmx { 

    class AuxInfoReader; 
    
    /**
     */
    class GDMLParser : public GeoParser { 

        public:

            /** Constructor. */
            GDMLParser(); 

            /** Destructor */ 
            ~GDMLParser(); 

            G4VPhysicalVolume* GetWorldVolume(); 

    
        private: 

            G4GDMLParser* parser_{new G4GDMLParser()};

            G4UImessenger* messenger_{nullptr}; 

            AuxInfoReader* auxInfoReader_{nullptr};  

    }; // GDMLParser 

} // ldmx

#endif // _SIM_APPLICATION_GEO_PARSER_H_ 
