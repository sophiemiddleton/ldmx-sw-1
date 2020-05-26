#!/usr/bin/python

from LDMX.Framework import ldmxcfg

hcalDigis = ldmxcfg.Producer("hcalDigis","ldmx::HcalDigiProducer")

#energy dep to amplitude multiplier
hcalDigis.parameters[ "gain" ] = 2000.;

#minimum amplitude on empty channels
hcalDigis.parameters[ "pedestal" ] = 1100.;

# padCapacitance to noiseRMS is assumed linear
hcalDigis.parameters[ "noiseIntercept" ] = 700.;
hcalDigis.parameters[ "noiseSlope" ] = 25.;

# capacitance of individual cells
hcalDigis.parameters[ "padCapacitance" ] = 0.1;

# MULTIPLE SAMPLES PER DIGI NOT IMPLEMENTED YET
# Just putting all info into SOI
hcalDigis.parameters[ "nADCs" ] = 10; # number of ADC samples to have per digi
hcalDigis.parameters[ "iSOI"  ] = 0; #index for the sample of interest

# threshold in multiples of noiseRMS as minimum to be readout
hcalDigis.parameters[ "readoutThreshold" ] = 4.;

# should I fill a configuration histogram?
hcalDigis.parameters[ "makeConfigHists" ] = False;
