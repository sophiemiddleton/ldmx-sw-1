
#ifndef EVENTPROC_ACTS_TEST_PROCESSOR_H_
#define EVENTPROC_ACTS_TEST_PROCESSOR_H_

//----------//
//   LDMX   //
//----------//
#include "Framework/EventProcessor.h"

namespace ldmx { 

    class ActsTestProcessor : public Producer { 
    
        public: 

            /** Constructor */
            ActsTestProcessor(const std::string &name, Process &process); 
            
            /** Destructor */
            ~ActsTestProcessor();

            /** 
             * Configure the processor using the given user specified parameters.
             * 
             * @param pSet Set of parameters used to configure this processor.
             */
            void configure(const ParameterSet &pSet);

            /**
             * Run the processor and create a collection of "digitized" Si
             * strip hits which include hit efficiency effects.
             *
             * @param event The event to process.
             */
            void produce(Event &event); 

        private: 

    }; // ActsTestProcessor
}

#endif // EVENTPROC_ACTS_TEST_PROCESSOR_H_
