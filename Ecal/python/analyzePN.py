
from LDMX.Framework import ldmxcfg

#####
# Analyzer that makes histograms focusing on PN showers in ECAL
# Default parameter values are commented out

analyzePN = ldmxcfg.Analyzer( "analyzePN" , "ldmx::AnalyzePN" );

# event object collection and pass names
analyzePN.parameters[ "simParticlesCollName" ] = "SimParticles"
analyzePN.parameters[ "simParticlesPassName" ] = "sim"

analyzePN.parameters[ "ecalDigiCollName" ] = "ecalDigis"
analyzePN.parameters[ "ecalDigiPassName" ] = "" #blank means take first match (any pass)

analyzePN.parameters[ "taggerSimHitsCollName" ] = "TaggerSimHits"
analyzePN.parameters[ "taggerSimHitsPassName" ] = "sim" 

analyzePN.parameters[ "hcalVetoCollName" ] = "HcalVeto"
analyzePN.parameters[ "hcalVetoPassName" ] = "" 

# minimum energy to allow photon to be categorized as primary
analyzePN.parameters[ "minPrimaryPhotonEnergy" ] = 2800. #MeV - matches trigger cut

# cuts on primary electron in last layer of tagger (min energy, max pT)
analyzePN.parameters[ "energyCut" ] = 3800. #MeV
analyzePN.parameters[ "pTCut"     ] = 100. #MeV

# cuts on event characteristics to be given storage hint "keep" (both are maxima)
analyzePN.parameters[ "lowReconEnergy" ] = 2000.0 #MeV
analyzePN.parameters[ "lowPNEnergy"    ] = 100.0 #MeV
