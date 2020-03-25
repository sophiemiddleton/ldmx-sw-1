#!/usr/bin/python

import sys

# we need the ldmx configuration package to construct the object
from LDMX.Framework import ldmxcfg

# first, we define the process, which must have a name which identifies this
# processing pass ("pass name").
p=ldmxcfg.Process("plot")  #sim") <-- wanted to run sim and dqm in one go but currently it crashes when the files are about to close.

# Currently, we need to explicitly identify plugin libraries which should be
# loaded.  In future, we do not expect this be necessary
p.libraries.append("libDQM.so")            # dqm processors                                     
p.libraries.append("libSimApplication.so") # simulation processor 

# load a simulator template to generate the simulated events                                                                                        
from LDMX.SimApplication.basicOneElectron import basicOneElectron
basicOneElectron.parameters[ "detector" ] = "detector.gdml" #need to provide path to detector description  (symlink)
basicOneElectron.parameters[ "runNumber" ] = 1 #need to provide a number identifying this sim run  
basicOneElectron.parameters[ "mpgVertex"     ] = [ -41.2642, 0., -865. ]  #mm
basicOneElectron.parameters[ "mpgMomentum"   ] = [ 380., 0., 3981.909 ]    #MeV                                                                    
# for some reason, setting these in the basicOneElectron example config isn't working for me


 
trigScintUp = ldmxcfg.Analyzer("TrigScintSimDQMUp", "ldmx::TrigScintDQM")
trigScintUp.parameters["hit_collection"] = "TriggerPadUpSimHits"
trigScintUp.parameters["pad"] = "up"

trigScintTag = ldmxcfg.Analyzer("TrigScintSimDQMTag", "ldmx::TrigScintDQM")
trigScintTag.parameters["hit_collection"] = "TriggerPadTaggerSimHits"
trigScintTag.parameters["pad"] = "tag"

trigScintDown = ldmxcfg.Analyzer("TrigScintSimDQMDown", "ldmx::TrigScintDQM")
trigScintDown.parameters["hit_collection"] = "TriggerPadDownSimHits"
trigScintDown.parameters["pad"] = "down"

# set the maximum number of events to process                                                                     
p.maxEvents=1000


# Define the sequence of event processors to be run
#p.sequence=[basicOneElectron,trigScintTag,trigScintUp,trigScintDown]
p.sequence=[trigScintTag,trigScintUp,trigScintDown]

# Provide the list of output files to produce
# if it can all be in one sequence, no input file is needed
#p.outputFiles=["ldmx_basicOneElectron_events.root"]
p.inputFiles=["ldmx_basicOneElectron_events.root"]   # assume this has been produced already, can be done separately, see settings as above 
p.histogramFile = "ldmx_basicOneElectron_events_simDqm.root" 


# Utility function to interpret and print out the configuration to the screen
p.printMe()
