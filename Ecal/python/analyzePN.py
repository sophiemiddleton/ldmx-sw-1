
from LDMX.Framework import ldmxcfg

#####
# Analyzer that makes histograms focusing on PN showers in ECAL
# Default parameter values are commented out

analyzePN = ldmxcfg.Analyzer( "analyzePN" , "ldmx::AnalyzePN" );

#analyzePN.parameters[ "simParticlesCollName" ] = "SimParticles"
#analyzePN.parameters[ "simParticlesPassName" ] = "sim"

#analyzePN.parameters[ "ecalDigiCollName" ] = "ecalDigis"
#analyzePN.parameters[ "ecalDigiPassName" ] = "" #blank means take first match (any pass)

#analyzePN.parameters[ "minPrimaryPhotonEnergy" ] = 2800. #MeV - matches trigger cut
#analyzePN.parameters[ "upstreamLossThresh" ] = 0.95 #min fraction of primary electron energy to allow left when reaching target

