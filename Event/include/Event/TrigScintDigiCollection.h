/**
 * @file TrigScintDigiCollection.h
 * @brief Class that represents a digitized hit in a trigger scintillator cell
 * @author Andrew Whitbeck, Texas Tech University
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef EVENT_TRIGSCINTDIGICOLLECTION_H_
#define EVENT_TRIGSCINTDIGICOLLECTION_H_

// LDMX
#include "Event/DigiCollection.h"

namespace ldmx {

    /**
     * @struct TrigScintDigiSample
     * @brief One sample of a Trigger Scintillator digi channel
     *
     * Usually several samples are used for each channel to re-construct the hit.
     */
    struct TrigScintDigiSample {
        //Note each sample is really two time samples
        //so that the information packs efficiently into 
        //the base digicollection class


        /** Raw integer ID of channel this sample is for */
        int rawID_{-1};
            
        /** ADC counts in this channel at this time */
        int adc_front_{0};
        int adc_back_{0};

        /** Time counts over threshhold in this channel */
        int tdc_front_{63};
        int tdc_back_{63};

        /** ID of capacitor used for integration in QIE */
        int cap_id_front_{0};
        int cap_id_back_{0};
    };

    /**
     * @class TrigScintDigiCollection
     * @brief Represents a collection of the trigger scintillator digi hits
     *
     * @note This class represents the digitized hit information
     * from the Trigger Scintillator
     */
    class TrigScintDigiCollection : public DigiCollection {

        public:

            /**
             * Class constructor.
             */
            TrigScintDigiCollection() {
            }

            /**
             * Class destructor.
             */
            virtual ~TrigScintDigiCollection() {
            }

            /**
             * Get samples for the input digi index
             */
            std::vector< TrigScintDigiSample > getDigi( unsigned int digiIndex ) const;

            /**
             * Translate and add samples to collection
             */
            void addDigi( std::vector< TrigScintDigiSample > newSamples );

        private:

            /** Number of bits used for the ADC */
            static const int ADC_NUM_BITS = 8;

            /** Mask for ADC bits */
            static const int ADC_BIT_MASK = (1<<ADC_NUM_BITS)-1;

            /** Bit position for ADC */
            static const int ADC_BIT_SHIFT = 0;
            
            /** Number of bits used for the TDC */
            static const int TDC_NUM_BITS = 6;

            /** Mask for TDC bits */
            static const int TDC_BIT_MASK = (1<<TDC_NUM_BITS)-1;

            /** Bit position for TDC */
            static const int TDC_BIT_SHIFT = ADC_NUM_BITS;

            /** Number of bits used for the Cap ID */
            static const int CID_NUM_BITS = 2;

            /** Mask for Cap ID bits */
            static const int CID_BIT_MASK = (1<<CID_NUM_BITS)-1;

            /** Bit position for Cap ID */
            static const int CID_BIT_SHIFT = ADC_NUM_BITS+TDC_NUM_BITS;

            /**
             * Get and Translate sample to the four measurements that could be encoded
             */
            TrigScintDigiSample getSample( unsigned int digiIndex , unsigned int sampleIndex ) const;

            /**
             * The ROOT class definition.
             */
            ClassDef(TrigScintDigiCollection, 1);
    };

}

#endif /* EVENT_TRIGSCINTDIGI_H_ */
