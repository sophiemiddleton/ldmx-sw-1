# Default settings for hcalHitMatcher

from LDMX.Framework import ldmxcfg

hcalHitMatcher = ldmxcfg.Analyzer( "hcalHitMatcher" , "ldmx::HcalHitMatcher" )

hcalHitMatcher.parameters[ "MinDepth_IncludeEventMaxPE" ] = 100.

hcalHitMatcher.parameters[ "ecalFrontZ" ]    = 200.
hcalHitMatcher.parameters[ "backZeroLayer" ] = 200. + 290.
hcalHitMatcher.parameters[ "sideZeroLayer" ] = 525./2.
