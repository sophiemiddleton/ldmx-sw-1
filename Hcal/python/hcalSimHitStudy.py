#!/usr/bin/python

# we need the ldmx configuration package to construct the object
from LDMX.Framework import ldmxcfg

hcalSimHitStudy = ldmxcfg.Analyzer("hcalSimHitStudy", "ldmx::HcalSimHitStudy")

# defaults are for v9 geometry
hcalSimHitStudy.parameters[ "backZeroLayer" ] = 200. + 290.
hcalSimHitStudy.parameters[ "sideZeroLayer" ] = 525./2.

hcalSimHitStudy.parameters[ "ecalFrontZ" ] = 200.

hcalSimHitStudy.parameters[ "knownPDGs" ] = [
            22 , #photon
            11 , #eletron
            -11 , #positron
            13 , #muon
            -13 , #anti-muon 
            2112 , #neutron 
            2212 , #proton 
            211 , -211 , 130 , 321 
        ]
