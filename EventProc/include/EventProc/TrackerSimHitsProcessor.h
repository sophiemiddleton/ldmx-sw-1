
#ifndef EVENTPROC_TRACKERSIMHITS_PROCESSOR_H_
#define EVENTPROC_TRACKERSIMHITS_PROCESSOR_H_

/***************/
/*   ldmx-sw   */
/***************/
#include "Framework/EventProcessor.h"

namespace ldmx { 

    /**
     *
     * This processor will loop over all of the Tracker hits (tagger or recoil) in an event, 
     * print out their details (if wanted) and store some hit info for tracking tests.
     * Should be extended for 3D hits formation
     */
    class TrackerSimHitsProcessor : public Producer { 
    
        public: 
        
            /** 
             * Constructor.
             *
             * @param name Name for this instance of the class.
             * @param process The Process class associated with EventProcessor, 
             * provided by the framework.
             */
            TrackerSimHitsProcessor(const std::string &name, Process &process); 

            /// Destructor
            ~TrackerSimHitsProcessor(); 

            /** 
             * Configure the processor using the given user specified parameters.
             * 
             * @param pSet Set of parameters used to configure this processor.
             */
            void configure(const ParameterSet &pSet);

            /**
             * Process the event and put new data products into it. 
             *
             * @param event The event to process.
             */
            void produce(Event &event); 
            
            
    private:

            //hit info - global coordinates
            float _x;
            float _y;
            float _z;
            float _sigma_x;
            float _sigma_y;
            float _sigma_z;
            //hit info - measurement frame
            float _u;
            float _sigma_u;
            
            
            std::string _collectionName;
            std::string _outputCollection;
    
    }; // TrackerSimHitsProcessor

} // ldmx

#endif // EVENTPROC_TRACKERSIMHITS_PROCESSOR_H_
