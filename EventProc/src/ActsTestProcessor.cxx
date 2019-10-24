
#include "EventProc/ActsTestProcessor.h"

namespace ldmx { 

    ActsTestProcessor::ActsTestProcessor(const std::string& name, Process& process) :
        Producer(name, process) { 
    }
    
    ActsTestProcessor::~ActsTestProcessor() { 
    }

    void ActsTestProcessor::configure(const ParameterSet &pSet) { 
    }

    void ActsTestProcessor::produce(Event& event) { 
    }       
}

DECLARE_PRODUCER_NS(ldmx, ActsTestProcessor) 
