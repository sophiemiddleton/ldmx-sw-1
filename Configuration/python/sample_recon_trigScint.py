#!/usr/bin/python
import sys

# we need the ldmx configuration package to construct the object
from LDMX.Framework import ldmxcfg

# first, we define the process, which must have a name which identifies this
# processing pass ("pass name").
p=ldmxcfg.Process("digi")

# Currently, we need to explicitly identify plugin libraries which should be
# loaded.  In future, we do not expect this be necessary
p.libraries.append("libEventProc.so")

# load the template hcalDigis configuration from its python file
from LDMX.EventProc.hcalDigis import hcalDigis

# switch to Strip aggregation (for testing)
hcalDigis.parameters["doStrip"] = 1

# load the template ecalDigis configuration from its python file
from LDMX.EventProc.ecalDigis import ecalDigis

# load the template ecalDigis configuration from its python file
from LDMX.EventProc.simpleTrigger import simpleTrigger

#load the template trigger scintillator digi configuration file
trigScintDigis = ldmxcfg.Producer("trigScintDigis", "ldmx::TrigScintDigiProducer")

trigScintDigis.parameters["meanNoise"] = 0.02
trigScintDigis.parameters["number_of_strips"] = 50
trigScintDigis.parameters["number_of_arrays"] = 3
trigScintDigis.parameters["mev_per_mip"] = 0.4
trigScintDigis.parameters["pe_per_mip"] = 10.
trigScintDigis.parameters["input_collection"]="TriggerPadUpSimHits"
trigScintDigis.parameters["output_collection"]="trigScintDigis"

trigScintDigisDn = ldmxcfg.Producer("trigScintDigisDn", "ldmx::TrigScintDigiProducer")

trigScintDigisDn.parameters["meanNoise"] = 0.02
trigScintDigisDn.parameters["number_of_strips"] = 50
trigScintDigisDn.parameters["number_of_arrays"] = 3
trigScintDigisDn.parameters["mev_per_mip"] = 0.4
trigScintDigisDn.parameters["pe_per_mip"] = 10.
trigScintDigisDn.parameters["input_collection"]="TriggerPadDownSimHits"
trigScintDigisDn.parameters["output_collection"]="trigScintDigisDn"

trigScintDigisTag = ldmxcfg.Producer("trigScintDigisTag", "ldmx::TrigScintDigiProducer")

trigScintDigisTag.parameters["meanNoise"] = 0.02
trigScintDigisTag.parameters["number_of_strips"] = 50
trigScintDigisTag.parameters["number_of_arrays"] = 3
trigScintDigisTag.parameters["mev_per_mip"] = 0.4
trigScintDigisTag.parameters["pe_per_mip"] = 10.
trigScintDigisTag.parameters["input_collection"]="TriggerPadTaggerSimHits"
trigScintDigisTag.parameters["output_collection"]="trigScintDigisTag"

# Define the sequence of event processors to be run
p.sequence=[trigScintDigis,trigScintDigisDn,trigScintDigisTag]

# Provide the list of input files to run on
p.inputFiles=["~/nfs/output.root"]

# Provide the list of output files to produce, either one to contain 
# the results of all input files or one output file name per input file name
p.outputFiles=["ldmx_digi_events.root"]

# Utility function to interpret and print out the configuration to the screen
p.printMe()

