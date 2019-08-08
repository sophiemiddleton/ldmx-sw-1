# Default settings for hcalHitMatcher

from LDMX.Framework import ldmxcfg

hcalHitMatcher = ldmxcfg.Analyzer( "hcalHitMatcher" , "ldmx::HcalHitMatcher" )

hcalHitMatcher.parameters[ "MinDepth_IncludeEventMaxPE" ] = 3

hcalHitMatcher.parameters[ "ecalFrontZ" ]    = 200.
