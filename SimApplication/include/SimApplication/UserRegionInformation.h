/**
 * @file UserRegionInformation.h
 * @brief Class which provides extra information for a detector region
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMAPPLICATION_USERREGIONINFORMATION_H_
#define SIMAPPLICATION_USERREGIONINFORMATION_H_

// Geant4
#include "G4VUserRegionInformation.hh"

namespace ldmx {

    /**
     * @class UserRegionInformation
     * @brief Defines extra information for a detector region
     *
     * @note
     * This extension to the user region information has a flag indicating
     * whether secondary particles should be stored.  This flag is used
     * in the UserTrackingAction to determine whether or not a trajectory
     * is created for a track created in the region.
     */
    class UserRegionInformation : public G4VUserRegionInformation {

        public:

            UserRegionInformation(bool storeSecondaries);

            virtual ~UserRegionInformation();

            void Print() const;

            bool getStoreSecondaries() const;

/**
     * Set the energy threshold.
     * @param[in] t The energy threshold.
     */
    void setThreshold(double t) {
        _threshold = t;
    }

    /**
     * Get the energy threshold.
     * @return The energy threshold.
     */
    double getThreshold() const {
        return _threshold;
    }


        private:

            bool storeSecondaries_;

            double _threshold{0}; 
    };

}

#endif
