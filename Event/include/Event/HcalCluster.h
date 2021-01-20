/**
 * @file HcalCluster.h
 * @brief Class that stores Stores reconstructed Cluster information from the HCAL
 * @author Sophie Middleton, Caltech
 */

#ifndef EVENT_HCALCLUSTER_H_
#define EVENT_HCALCLUSTER_H_

// LDMX
#include "Event/HcalHit.h"

namespace ldmx {

    /**
     * @class HcalCluster
     * @brief Stores reconstructed cluster information from the HCAL
     *
     */
    class HcalCluster  {

        public:

            /**
             * Class constructor.
             */
            HcalCluster() {}

            /**
             * Class destructor.
             */
            virtual ~HcalCluster() {}

            /**
             * Clear the data in the object.
             */
            void Clear();

            /**
             * Print out the object.
             */
            void Print() const;

            /**
             * Get the number of hits in cluster.
             * @return Number of hits in cluster
             */
           // float getNHits() const {
           //     return nhits;
           // }

            /**
             * Get the total energy in cluster
             * @returntotal energy in cluster
             */
            //float getTotalEdep const {
            //    return TotalEdep;
            //}            

          

        private:
            unsigned int nhits;
            double TotalEdep;
            //std::vector<HcalHit> HitList;
            ClassDef(HcalCluster, 2);
    };

}

#endif /* EVENT_HCALHIT_H_ */
