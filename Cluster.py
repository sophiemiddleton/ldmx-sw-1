import os

from LDMX.Framework import ldmxcfg
from LDMX.SimCore import generators
from LDMX.SimCore import simulator

p=ldmxcfg.Process("v12")

from LDMX.Ecal import digi
from LDMX.Ecal import vetos
from LDMX.Hcal import hcal
from LDMX.DetDescr.HcalGeometry import HcalGeometry
from LDMX.Hcal import HcalGeometry
from LDMX.Tools.HgcrocEmulator import HgcrocEmulator
# Instantiate the simulator
sim = simulator.simulator("signal")

# Set the detector to use and enable the scoring planes
sim.setDetector( 'ldmx-det-v12', True)

# Set the run number
#p.runNumber = {{ run }}

# Set a description of what type of run this is.
sim.description = "Signal generated using the v12 detector."

# Set the random seeds
sim.randomSeeds = [ 1,2 ]

# Smear the beamspot 
sim.beamSpotSmear = [ 20., 80., 0 ]

# Enable the LHE generator
sim.generators.append(generators.lhe( "Signal Generator", ("WAB_FF3.lhe" ))) 




#hcalDigis = digi.HcalDigiProducer()
#hcalDigis.hgcroc.noise = False
hcalClusters = hcal.HcalClusterProducer()
hcalOldDigis = hcal.HcalOldDigiProducer()

#hcalrec = hcal.HcalRecProducer()
#hcalVeto  = hcal.HcalVetoProcessor()

geom = HcalGeometry.HcalGeometryProvider.getInstance()


p.sequence=[ sim, hcalOldDigis,hcalClusters]

p.outputFiles = [ "test.root"]

p.maxEvents = 100
p.logFrequency = 10
p.lheFilePath = ("WAB_FF3.lhe" )
p.pause()
