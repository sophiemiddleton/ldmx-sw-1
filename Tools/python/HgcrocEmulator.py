"""Configuration for HGCROC Emulator"""

class HgcrocEmulator() :
    """Configuration for HGCROC Emulator

    Attributes
    ----------
    pedestal : float
        Basline readout ADC counts of chip
    clockCycle : float
        Cycle of chip clock [ns]
    measTime : float
        Time that chip takes voltage measurement within clock window [ns]
    timingJitter : float
        Uncertainty in chip clock [ns]
    readoutPadCapacitance : float
        Capacitance [pF] of chip readout pad
    maxADCRange : float
        Maximum charge that chip can readout [fC]
    nADCs : int
        Number of voltage samples to measure for one DIGI
    iSOI : int
        (UNUSED) Index for sample of interest within multi-sample DIGI
    gain : float
        Conversion from ADC Counts to voltage [mV]
    noiseRMS : float
        Average noise within chip [mV]
    readoutThreshold : float
        Threshold [mV] for chip to construct a DIGI from a voltage signal
    toaThreshold : float
        Threshold [mV] for chip to measure Time Of Arrival
    totThreshold : float
        Threshold [mV] for chip to go into saturation and measure Time Over Threshold
    drainRate : float
        Rate that chip drains during saturation [mV/ns]
    """

    def __init__(self) :

        #######################################################################
        # Settings of the chip
        self.pedestal = 50. #ADC counts - baseline factor to subtract off of readout
        self.clockCycle = 25.0 #ns
        self.measTime = 0. #ns
        self.timingJitter = self.clockCycle / 100. #ns - pretty arbitrarily chosen
        self.readoutPadCapacitance = 20. #pF <- derived from hardware of HGCROC
        self.maxADCRange = 320. #fC <- setting of HGCROC
        self.nADCs = 10 
        self.iSOI  = 0 
        self.totMax = 200. #ns - maximum TOT allowed by chip (spec sheet)
        self.drainRate = 10240. / self.totMax #fC / ns - rate charge drains off during TOT (spec sheet, is tune-able)
        self.rateUpSlope =  -0.345
        self.timeUpSlope = 70.6547
        self.rateDnSlope = 0.140068
        self.timeDnSlope = 87.7649
        self.timePeak    = 77.732

        #Voltage -> ADC Counts conversion
        # voltage [mV] / gain = ADC Counts
        #
        # gain = maximum ADC range [fC] ( 1 / readout pad capacitance in pF ) ( 1 / 2^10 ADC Counts ) = mV / ADC counts
        self.gain = self.maxADCRange/self.readoutPadCapacitance/1024 # mV / ADC

        #######################################################################
        # Physical Constants for Detector Materials

        self.noiseRMS         = 0. #mV - useless default
        self.setNoise( 700. , 25. ) #depends on readoutPadCapacitance

        self.readoutThreshold = self.pedestal + 2. #ADC Counts

        self.toaThreshold     = 0. #mV - useless default
        self.totThreshold     = 0. #mV - useless default

        # turn on or off noise
        #   NOT DOCUMENTED - only meant for testing purposes
        self.noise = True

    def calculateVoltage(self, electrons) :
        """Calculate the voltage signal [mV] of the input number of electrons

        Uses the charge of 1000 electrons in fC and the capacitance of the readout pads.

        electrons ( 0.162 fC / 1000 electrons ) ( 1 / capacitance in pF ) = voltage [mV]

        Parameters
        ----------
        electrons : int
            Number of electrons (or e-h pairs) produced
        """

        return electrons*(0.162/1000.)*(1./self.readoutPadCapacitance)
    
    def calculateVoltageHcal(self, PE) :
        """ 
        Assuming that 1 PEs ~ 5mV (before it was  7 PEs ~ 2.5V)
        Parameters
        ----------
        PE : int                                                                                                                                                              
            Number of photo-electrons produced  
        """
        return PE*(5/1)

    def setNoise(self, noiseIntercept , noiseSlope ) :
        """Calculate the Noise RMS [mV] from the capacitance of the readout pads.

        Parameters
        ----------
        noiseIntercept : float
            Noise when there is no capacitance
        noiseSlope : float
            Ratio of noise in electrons to capacitance in pF of pads
        """

        self.noiseRMS = self.calculateVoltage(noiseIntercept + noiseSlope*self.readoutPadCapacitance)

    def setThresholdDefaults(self, nElectronsPerMIP):
        """Set the different thresholds of the chip

        The default calculation for the different thresholds if the following:
        - toa is 5 MIPs above the pedestal
        - tot is 50 MIPs above the pedestal

        These calculations depend on the following parameters;
        the user should call this function _after_ setting these parameters.
        - pedestal
        - gain
        - readoutPadCapacitance

        Parameters
        ----------
        nElectronsPerMIP : int
            Number of electrons generated by a MIP in the detector
        """

        self.toaThreshold = self.gain*self.pedestal + self.calculateVoltage( 5.*nElectronsPerMIP )
        self.totThreshold = self.gain*self.pedestal + self.calculateVoltage( 50.*nElectronsPerMIP )

    def setThresholdDefaultsHcal(self):
        self.pedestal = 25. # ADC
        # gain = maximum ADC range [fC] ( 1 / readout pad capacitance in pF ) ( 1 / 2^10 ADC Counts ) = mV / ADC counts
        # if gain = 0.4 and cap = 20 pF => maxADCrange = 8192 = 8pC
        # if gain = 0.1 and cap = 20 pF => maxADCrange = 2pC
        self.maxADCRange = 8192. # fC
        self.gain = 0.1 # mV/ADC
        #self.readoutThreshold = self.gain*self.pedestal + self.calculateVoltageHcal(1) # [mV] # readout threshold 1 PE -> 5 mV
        self.readoutThreshold = self.pedestal + self.calculateVoltageHcal(1)/self.gain # readout threshold 1 PE -> 5 mV -> 62.5 ADC(?) -> goes to int
        self.toaThreshold     = self.gain*self.pedestal + self.calculateVoltageHcal(0.1*68) # toa 0.1 MIP + ped*gain -> if gain=0.015, ped=50 -> theshold 34.75 [mV]
        self.totThreshold     = self.gain*self.pedestal + self.calculateVoltageHcal(2*68) # tot 2 MIPs
