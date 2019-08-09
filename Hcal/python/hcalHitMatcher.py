# Default settings for hcalHitMatcher

from LDMX.Framework import ldmxcfg

hcalHitMatcher = ldmxcfg.Analyzer( "hcalHitMatcher" , "ldmx::HcalHitMatcher" )

hcalHitMatcher.parameters[ "minLayerEventMaxPE" ] = 3

hcalHitMatcher.parameters[ "ecalFrontZ" ]    = 200.
