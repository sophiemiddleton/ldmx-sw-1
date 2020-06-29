# need the configuration python module
from LDMX.Framework import ldmxcfg

# Create the necessary process object (call this pass "sim")
p = ldmxcfg.Process( "sim" )

# Define necessary libraries that need to be loaded for these processors
p.libraries = [
    #"libEventProc.so", #to load hcalDigis
    "libEcal.so", #to load Ecal processors
    "libHcal.so", #to load Hcal processors
    "libSimApplication.so" #to load the Simulator
]

# import a template simulator and change some of its parameters
from LDMX.SimApplication import examples as ex

# import template reconstruction processors
from LDMX.Ecal.ecalDigis import ecalDigis
from LDMX.Ecal.ecalRecon import ecalRecon
from LDMX.Hcal.hcalDigis import hcalDigis

# tell the process what sequence to do the processors in
p.sequence = [
    ex.inclusive_single_e(),
    ecalDigis,
    hcalDigis
]

# During production (simulation), maxEvents is used as the number
# of events to simulate.
# Other times (like when analyzing a file), maxEvents is used as
# a cutoff to prevent ldmx-app from running over the entire file.
p.maxEvents = 10

# how frequently should the process print messages to the screen?
p.logFrequency = 1

# give name of output file
p.outputFiles = [ "output.root" ]

# print process object to make sure configuration is correct
# at beginning of run
print p
