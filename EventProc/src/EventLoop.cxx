#include "EventProc/EventLoop.h"

// STL
#include <iostream>
#include <stdexcept>

namespace eventproc {

void EventLoop::initialize() {
    for (EventProcessor* processor : processors_) {
        processor->setEvent(eventFile_->getEvent()); 
        processor->initialize(); 
    }
}

void EventLoop::run(int nEvents) {
    int nProcessed = 0;
    while (eventFile_->nextEvent()) {
        //std::cout << "Event: " << nProcessed << std::endl;
        for (EventProcessor* processor : processors_) {
            processor->execute(); 
        }
        ++nProcessed;
        if (nEvents > 0 && nProcessed >= nEvents) {
            break;
        }
    }
    std::cout << "EventLoop: Finished processing " << nProcessed << " events out of "
            << nEvents << " requested." << std::endl;
}

void EventLoop::finish() {
    for (EventProcessor* processor : processors_) {
       processor->finish(); 
    }
}

void EventLoop::setEventSource(event::EventFile* eventSource) { 
    eventFile_ = eventSource;
}

}
