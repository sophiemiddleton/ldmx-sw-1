import os

from LDMX.Framework import ldmxcfg
from LDMX.SimCore import generators
from LDMX.SimCore import simulator

p=ldmxcfg.Process("v12")
p.libraries.append("libSimCore.so")
p.libraries.append("libHcal.so")
p.libraries.append("libEcal.so")


from LDMX.SimCore import simulator
from LDMX.SimCore import generators

# Instantiate the simulator
sim = simulator.simulator("signal")

# Set the detector to use and enable the scoring planes
sim.setDetector( 'ldmx-det-v12', True)

# Set a description of what type of run this is.
sim.description = "Signal generated using the v12 detector."

# Set the random seeds
sim.randomSeeds = [ 1,2 ]

# Smear the beamspot 
sim.beamSpotSmear = [ 20., 80., 0 ]

# Enable the LHE generator
sim.generators.append(generators.lhe( "Signal Generator", ("WAB_FF3.lhe" ))) 

from LDMX.Ecal import EcalGeometry
geom = EcalGeometry.EcalGeometryProvider.getInstance()

from LDMX.Hcal import HcalGeometry
geom = HcalGeometry.HcalGeometryProvider.getInstance()

#from LDMX.Hcal import digi
#hcaldigi = digi.HcalDigiProducer()
#hcaldigi.hgcroc.noise = False

from LDMX.Hcal import hcal
hcalolddigi = hcal.HcalOldDigiProducer()
hcalcluster = hcal.HcalClusterProducer()

#hcalrec = digi.HcalRecProducer()
p.sequence=[sim,hcalolddigi,hcalcluster]


p.outputFiles = [ "test.root"]
p.maxEvents = 100
p.logFrequency = 10
p.lheFilePath = ("WAB_FF3.lhe" )



