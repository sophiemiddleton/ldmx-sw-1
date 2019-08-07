#!/usr/bin/python

# we need the ldmx configuration package to construct the object
from LDMX.Framework import ldmxcfg

hcalSimHitStudy = ldmxcfg.Analyzer("hcalSimHitStudy", "ldmx::HcalSimHitStudy")

# defaults are for v9 geometry
hcalSimHitStudy.parameters[ "backZeroLayer" ] = 200. + 290.
hcalSimHitStudy.parameters[ "sideZeroLayer" ] = 525./2.

hcalSimHitStudy.parameters[ "ecalFrontZ" ] = 200.
