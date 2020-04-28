#include "Event/TrigScintDigiCollection.h"

ClassImp(ldmx::TrigScintDigiCollection)

namespace ldmx {

    std::vector< TrigScintDigiSample > TrigScintDigiCollection::getDigi( unsigned int digiIndex ) const {
        
        std::vector< TrigScintDigiSample > digi;
        for ( unsigned int sampleIndex = 0; sampleIndex < this->getNumSamplesPerDigi(); sampleIndex++ ) {
            digi.push_back( this->getSample( digiIndex , sampleIndex ) );
        }

        return digi;
    }

    void TrigScintDigiCollection::addDigi( std::vector< TrigScintDigiSample > newSamples ) {
        
        std::vector< TrigScintDigiSample >::iterator it_newSamples;
        int channelID = newSamples.at(0).rawID_;
        std::vector< int32_t > words;
        for ( it_newSamples = newSamples.begin(); it_newSamples != newSamples.end(); ++it_newSamples ) {
            
            //check for bit overflow...
            int adc_front = (it_newSamples->adc_front_ > ADC_BIT_MASK) ? ADC_BIT_MASK : it_newSamples->adc_front_;
            int adc_back = (it_newSamples->adc_back_ > ADC_BIT_MASK) ? ADC_BIT_MASK : it_newSamples->adc_back_;
            int tdc_front   = (it_newSamples->tdc_front_   > TDC_BIT_MASK) ? TDC_BIT_MASK : it_newSamples->tdc_front_;
            int tdc_back   = (it_newSamples->tdc_back_   > TDC_BIT_MASK) ? TDC_BIT_MASK : it_newSamples->tdc_back_;
            int cid_front   = (it_newSamples->cap_id_front_   > CID_BIT_MASK) ? CID_BIT_MASK : it_newSamples->cap_id_front_;
            int cid_back   = (it_newSamples->cap_id_back_   > CID_BIT_MASK) ? CID_BIT_MASK : it_newSamples->cap_id_back_;

            // pack...
            word = ( cid_front << (CID_BIT_SHIFT+16)  ) +
              ( tdc_front << (TDC_BIT_SHIFT+16) ) +
              ( adc_front << (ADC_BIT_SHIFT+16) ) + 
              ( cid_back << CID_BIT_SHIFT  ) +
              ( tdc_back << TDC_BIT_SHIFT ) +
              ( adc_back << ADC_BIT_SHIFT ) ;

            words.push_back( word );
        }

        DigiCollection::addDigi( channelID , words );

        return;
    }

    TrigScintDigiSample TrigScintDigiCollection::getSample( unsigned int digiIndex , unsigned int sampleIndex ) const {

        TrigScintDigiSample sample;

        sample.rawID_ = this->getChannelID( digiIndex );

        int32_t word = DigiCollection::getSampleWord( digiIndex , sampleIndex  );

        //this is where the word --> measurements translation occurs
        sample.adc_front_    = ADC_BIT_MASK & ( word >> (ADC_BIT_SHIFT+16) );
        sample.tdc_front_    = TDC_BIT_MASK & ( word >> (TDC_BIT_SHIFT+16) );
        sample.cap_id_front_ = CID_BIT_MASK & ( word >> (CID_BIT_SHIFT+16) );
        sample.adc_back_    = ADC_BIT_MASK & ( word >> (ADC_BIT_SHIFT) );
        sample.tdc_back_    = TDC_BIT_MASK & ( word >> (TDC_BIT_SHIFT) );
        sample.cap_id_back_ = CID_BIT_MASK & ( word >> (CID_BIT_SHIFT) );

        return sample;
    }
}
