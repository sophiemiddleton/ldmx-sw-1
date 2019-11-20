
#include "EventProc/TrackerSimHitsProcessor.h" 

/************/
/*   LDMX   */
/************/
#include "Event/SimTrackerHit.h"

namespace ldmx { 

    TrackerSimHitsProcessor::TrackerSimHitsProcessor(const std::string &name, Process &process) : 
        Producer(name, process) { 
    }

    TrackerSimHitsProcessor::~TrackerSimHitsProcessor() { 
    }

    void TrackerSimHitsProcessor::configure(const ParameterSet &pSet) { 
        _collectionName = pSet.getString("collectionName");
    }

    void TrackerSimHitsProcessor::produce(Event& event) { 
        
        if (!event.exists(_collectionName)) {
            std::cout<<"ERROR:: looking for non-existing collection "<< _collectionName<<std::endl;
            return; 
        }
        
        // Get the collection of digitized ECal hits from the event
        auto hits = event.getCollection(_collectionName); 

        // Loop over the collection of hits and print the hit details
        for (int iHit{0}; iHit < hits->GetEntriesFast(); ++iHit) { 
            
            // Retrieve the ith hit from the collection
            auto hit = static_cast<SimTrackerHit*>(hits->At(iHit)); 

            // Print the hit
            hit->Print(); 
        }
    }
} // ldmx

DECLARE_PRODUCER_NS(ldmx, TrackerSimHitsProcessor) 
