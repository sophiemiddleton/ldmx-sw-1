/**
 * @file EcalDigiCollection.h
 * @brief Class that represents a digitized hit in a calorimeter cell within the ECal
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef EVENT_ECALDIGICOLLECTION_H_
#define EVENT_ECALDIGICOLLECTION_H_

// ROOT
#include "TObject.h" //for ClassDef

// STL
#include <stdint.h> //32bit words
#include <vector> //vector lists
#include <iostream> //Print method

namespace ldmx {

    /**
     * @struct EcalDigiSample
     * @brief One sample of an Ecal digi channel corresponding to one clock of the HGCROC chip
     *
     * Not all of these measurements are valid in each sample.
     * The valid measurements depend on the tot_progress_ and tot_complete_ flags.
     *
     * The toa_ measurement is always valid and is inserted as the third measurement in the 32-bit word.
     *
     * If the TOT measurment is NOT complete, then the other
     * two valid measurements (in order) are
     *  1. ADC of the previous sample (adc_tm1_)
     *  2. ADC of this sample (adc_t_)
     *
     * If the TOT is NOT in progress and the TOT is complete, then
     *  1. ADC of the previous sample (adc_tm1_)
     *  2. TOT measurement (tot_)
     *
     * If both flags are true, then
     *  1. ADC of this sample (adc_t_)
     *  2. TOT measurement (tot_)
     *
     * Usually several samples are used for each channel to re-construct the hit.
     */
    struct EcalDigiSample {
        /** Raw integer ID of channel this sample is for */
        int rawID_{-1};

        /** ADC counts in this channel at this clock */
        int adc_t_{-1};

        /** ADC counts in this channel at the previous clock */
        int adc_tm1_{-1};

        /** Time counts over threshhold in this channel during this clock */
        int tot_{-1};

        /** Time counts when signal arrived in this channel during this clock */
        int toa_{-1};

        /** Is the TOT measurement in progress during this sample? */
        bool tot_progress_{false};

        /** Is the TOT measurement complete at this sample? */
        bool tot_complete_{false};
    };

    /**
     * @class EcalDigiCollection
     * @brief Represents a collection of the ECal digi hits
     *
     * @note This class represents the digitized signal information
     * in the form of a series of samples for each channel of readout.
     * Each channel is represented by an ID integer and each sample is a 32-bit word.
     * The number of samples for each digi is configurable, but is required to be
     * the same for all channels.
     *
     * Each digi corresponds to a one channel ID and numSamplesPerDigi_ samples.
     */
    class EcalDigiCollection {

        public:

            /**
             * Class constructor.
             */
            EcalDigiCollection() { }

            /**
             * Class destructor.
             */
            virtual ~EcalDigiCollection() { }

            /**
             * Clear the data in the object.
             *
             * Clears the vectors of channel IDs and samples, but does not change the number of samples per digi setting.
             */
            void Clear();

            /**
             * Print out the object.
             *
             * Prints out the lengths of the stored vectors and the number of samples per digi setting.
             */
            void Print() const;

            /**
             * Get number of samples per digi 
             */
            unsigned int getNumSamplesPerDigi() const { return numSamplesPerDigi_; }

            /**
             * Set number of samples for each digi
             */
            void setNumSamplesPerDigi( unsigned int n ) { numSamplesPerDigi_ = n; return; }

            /**
             * Get index of sample of interest
             */
            unsigned int getSampleOfInterestIndex() const { return sampleOfInterest_; }

            /**
             * Set index of sample of interest
             */
            void setSampleOfInterestIndex( unsigned int n ) { sampleOfInterest_ = n; return; }
            
            /**
             * Get samples for the input digi index
             *
             * @note This is where the 32-bit word to measurements translation occurs.
             *
             * Each "digi" is numSamplesPerDigi_ samples.
             * The sample is a single 32-bit word that is then translated into
             * a EcalDigiSample depending on the first two bits.
             *
             * The first two bits are interpreted as:
             *  1. Whether TOT measurement is in progress during this sample
             *  2. TOT measurement is complete at this sample
             *
             * After the first two bits, the next 30 bits are broken into 
             * three 10 bit measurements. The first two measurments depend
             * on the two flags above. The third measurement is set to be
             * the time of arrival measurement (TOA).
             *
             * @sa EcalDigiSample for how the valid measurements depend on the flags.
             *
             * Any measurements that aren't set becuase of the interpretation
             * from the flags are left as their default value.
             */
            std::vector< EcalDigiSample > getDigi( unsigned int digiIndex ) const;

            /**
             * Get total number of digis
             */
            unsigned int getNumDigis() const { return channelIDs_.size(); }

            /**
             * Translate and add samples to collection
             *
             * @note This is where the measurements to 32-bit word translation occurs.
             *
             * @sa EcalDigiSample for how the valid measurements depend on the flags.
             *
             * Any measurements that aren't used as indicated by the flags
             * are ignored and not saved into the word.
             */
            void addDigi( std::vector< EcalDigiSample > newSamples );

        private:

            /** Mask for lowest order bit in an int */
            static const int ONE_BIT_MASK = 1;

            /** Mask for lowest order ten bits in an int */
            static const int TEN_BIT_MASK = (1 << 10) - 1;

            /** Bit position of first flag */
            static const int FIRSTFLAG_POS = 31;

            /** Bit position of second flag */
            static const int SECONFLAG_POS = 30;

            /** Bit position of first measurement */
            static const int FIRSTMEAS_POS = 20;

            /** Bit position of second measurement */
            static const int SECONMEAS_POS = 10;

        private:

            /** list of channel IDs that we have digis for */
            std::vector< int > channelIDs_;

            /** list of samples that we have been given */
            std::vector< int32_t > samples_;

            /** number of samples for each digi */
            unsigned int numSamplesPerDigi_{1};

            /** index for the sample of interest in the samples list */
            unsigned int sampleOfInterest_{0};

            /**
             * The ROOT class definition.
             */
            ClassDef(EcalDigiCollection, 1);
    };

}

#endif /* EVENT_ECALDIGI_H_ */
