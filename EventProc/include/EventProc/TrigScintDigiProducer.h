/**
 * @file TrigScintDigiProducer.h
 * @brief Class that performs digitization of simulated trigger sctintillator
 * @author Andrew Whitbeck, TTU
 */

#ifndef EVENTPROC_TRIGSCINTDIGIPRODUCER_H_
#define EVENTPROC_TRIGSCINTDIGIPRODUCER_H_

// C++/STL
#include <time.h>

// ROOT
#include "TString.h"
#include "TRandom3.h"

// LDMX
#include "DetDescr/DefaultDetectorID.h"
#include "Event/EventConstants.h"
#include "Event/TrigScintHit.h"
#include "Event/SimCalorimeterHit.h"
#include "Framework/EventProcessor.h"
#include "Tools/NoiseGenerator.h"

namespace ldmx {

    enum TrigScintSection{UPSTREAM_TAGGER,UPSTREAM_TARGET,DOWNSTREAM_TARGET,NUM_SECTIONS};

    /**
     * @class TrigScintDigiProducer
     * @brief Performs digitization of simulated Trigger Scintillator data
     */
    class TrigScintDigiProducer : public Producer {

        public:

            typedef int layer;

            typedef std::pair<double, double> zboundaries;

            TrigScintDigiProducer(const std::string& name, Process& process);

            virtual ~TrigScintDigiProducer() {
                delete hits_;
                if (random_)
                    delete random_;
            }

            virtual void configure(const ParameterSet&);

            virtual void produce(Event& event);

            unsigned int generateRandomID(TrigScintSection sec);

        private:

            TClonesArray* hits_{nullptr};
            TRandom3* random_{new TRandom3(time(nullptr))};
            bool verbose_{false};
            DefaultDetectorID* detID_{nullptr};
	    std::string input_collection_;
	    std::string output_collection_;
            NoiseGenerator* noiseGenerator_;

            /** Generator for simulating noise hits. */
            double meanNoise_{0};
            int    nProcessed_{0};
            double mev_per_mip_{1.40};
            double pe_per_mip_{13.5};
	    int    NUM_STRIPS_PER_ARRAY_{50};
            int    NUM_ARRAYS_{3};
    };

}

#endif
