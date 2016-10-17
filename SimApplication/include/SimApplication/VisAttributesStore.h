#ifndef SimApplication_VisAttributesStore_h
#define SimApplication_VisAttributesStore_h

// Geant4
#include "G4VisAttributes.hh"

class VisAttributesStore {

    public:

        typedef std::map<std::string, G4VisAttributes*> VisAttributesMap;

        static VisAttributesStore* getInstance() {
            static VisAttributesStore INSTANCE;
            return &INSTANCE;
        }

        G4VisAttributes* getVisAttributes(const std::string& name) {
            return visAttributesMap[name];
        }

        void addVisAttributes(const std::string& name, G4VisAttributes* visAttributes) {
            visAttributesMap[name] = visAttributes;
        }

    private:

        VisAttributesMap visAttributesMap;
};

#endif
