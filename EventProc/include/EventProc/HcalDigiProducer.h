/**
 * @file HcalDigiProducer.h
 * @brief Class that performs digitization of simulated HCal data
 * @author Andrew Whitbeck, FNAL
 */

#ifndef EVENTPROC_HCALDIGIPRODUCER_H_
#define EVENTPROC_HCALDIGIPRODUCER_H_

// C++/STL
#include <time.h>

// ROOT
#include "TString.h"
#include "TRandom3.h"

// LDMX
#include "DetDescr/DetectorID.h"
#include "Event/SimCalorimeterHit.h"
#include "Framework/EventProcessor.h"
#include "Tools/NoiseGenerator.h"

namespace ldmx {

    /**
     * @class HcalDigiProducer
     * @brief Performs digitization of simulated HCal data
     */
    class HcalDigiProducer : public Producer {

        public:

            typedef int layer;

            typedef std::pair<double, double> zboundaries;

            HcalDigiProducer(const std::string& name, Process& process);

            virtual ~HcalDigiProducer() {
                delete hits_;
                if (random_)
                    delete random_;
            }

            virtual void configure(const ParameterSet&);

            virtual void produce(Event& event);

            unsigned int generateRandomID(bool isBackSection);

        private:

            TClonesArray* hits_{nullptr};
            TRandom3* random_{new TRandom3(time(nullptr))};
            std::map<layer, zboundaries> hcalLayers_;
            bool verbose_{false};
            DetectorID* detID_{nullptr};
            
            /** Generator for simulating noise hits. */
            NoiseGenerator* noiseGenerator_;

            double meanNoise_{0};
            int    nProcessed_{0};
            double mev_per_mip_{1.40};
            double pe_per_mip_{13.5};
            int    readoutThreshold_{3};
            int    doStrip_{true};
            int    STRIPS_BACK_PER_LAYER_{20};
            int    NUM_BACK_HCAL_LAYERS_{100};
            int    STRIPS_SIDE_PER_LAYER_{20};
            int    NUM_SIDE_HCAL_LAYERS_{100};
    };

}

#endif
