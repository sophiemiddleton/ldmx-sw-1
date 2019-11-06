#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"

#include "EventProc/TrigScintDigiProducer.h"

#include <iostream>
#include <exception>

namespace ldmx {

    TrigScintDigiProducer::TrigScintDigiProducer(const std::string& name, Process& process) :
        Producer(name, process) {
        hits_ = new TClonesArray("ldmx::TrigScintHit");
        noiseGenerator_ = new NoiseGenerator();
    }

    void TrigScintDigiProducer::configure(const ParameterSet& ps) {
        detID_       = new DefaultDetectorID();
        random_       = new TRandom3(ps.getInteger("randomSeed", 1000));
        NUM_STRIPS_PER_ARRAY_      = ps.getInteger("number_of_strips");
        NUM_ARRAYS_                = ps.getInteger("number_of_arrays");
        meanNoise_                 = ps.getDouble("meanNoise");
        mev_per_mip_               = ps.getDouble("mev_per_mip");
        pe_per_mip_                = ps.getDouble("pe_per_mip");
	input_collection_          = ps.getString("input_collection","TriggerPadUpSimHits");
	output_collection_          = ps.getString("output_collection","trigScintDigis");
	noiseGenerator_ = new NoiseGenerator(meanNoise_,false); 
	noiseGenerator_->setNoiseThreshold(1); 
    }

    unsigned int TrigScintDigiProducer::generateRandomID(TrigScintSection sec){
        DefaultDetectorID tempID;
        if( sec < TrigScintSection::NUM_SECTIONS ){
	    tempID.setFieldValue(0,int(sec));
	    tempID.setFieldValue(1,random_->Integer(NUM_STRIPS_PER_ARRAY_));
	}else
	    std::cout << "WARNING [TrigScintDigiProducer::generateRandomID]: TrigScintSection is not known" << std::endl;

        return tempID.pack();
    }

    void TrigScintDigiProducer::produce(Event& event) {

        std::map<unsigned int, int>   cellPEs;
        std::map<unsigned int, int>   cellMinPEs;
        std::map<unsigned int, float> Xpos, Ypos, Zpos, Edep, Time;
        std::unordered_set<unsigned int> noiseHitIDs;
	int numRecHits=0;

        // looper over sim hits and aggregate energy depositions for each detID
        TClonesArray* simHits = (TClonesArray*) event.getCollection(input_collection_, "sim");

        int numSimHits = simHits->GetEntries();
        for (int iHit = 0; iHit < numSimHits; iHit++) {            
            SimCalorimeterHit* simHit = (SimCalorimeterHit*) simHits->At(iHit);
             int detIDraw = simHit->getID();
            detID_->setRawValue(detIDraw);
            detID_->unpack();
	    std::vector<float> position = simHit->getPosition();

            if (verbose_) {
                std::cout << "section: " << detID_->getFieldValue("section") << "  layer: " << detID_->getFieldValue("layer") <<  "  strip: " << detID_->getFieldValue("strip") <<std::endl;
            }        
            
            // for now, we take am energy weighted average of the hit in each stip to simulate the hit position. 
            // will use strip TOF and light yield between strips to estimate position.            
            if (Edep.find(detIDraw) == Edep.end()) {
                // first hit, initialize
                Edep[detIDraw] = simHit->getEdep();
                Time[detIDraw] = simHit->getTime() * simHit->getEdep();
                Xpos[detIDraw]      = position[0]* simHit->getEdep();
                Ypos[detIDraw]      = position[1]* simHit->getEdep();
                Zpos[detIDraw]      = position[2]* simHit->getEdep();
		numRecHits++;
            } else {
                // not first hit, aggregate, and store the largest radius hit
                Xpos[detIDraw]      += position[0]* simHit->getEdep();
                Ypos[detIDraw]      += position[1]* simHit->getEdep();
                Zpos[detIDraw]      += position[2]* simHit->getEdep();
                Edep[detIDraw]      += simHit->getEdep();
		// AJW: need to figure out a better way to model this...
                Time[detIDraw]      += simHit->getTime() * simHit->getEdep();
            }
	    
        }

        // loop over detIDs and simulate number of PEs
        int ihit = 0;        
        for (std::map<unsigned int, float>::iterator it = Edep.begin(); it != Edep.end(); ++it) {
            int detIDraw = it->first;
            double depEnergy    = Edep[detIDraw];
            Time[detIDraw]      = Time[detIDraw] / Edep[detIDraw];
            Xpos[detIDraw]      = Xpos[detIDraw] / Edep[detIDraw];
            Ypos[detIDraw]      = Ypos[detIDraw] / Edep[detIDraw];
            Zpos[detIDraw]      = Zpos[detIDraw] / Edep[detIDraw];
            double meanPE       = depEnergy / mev_per_mip_ * pe_per_mip_;
	    cellPEs[detIDraw]   = random_->Poisson(meanPE);

	    //readout threshold
	    if( cellPEs[detIDraw] >= 1 ){ // > or >= ?
                
	      TrigScintHit *hit = (TrigScintHit*) (hits_->ConstructedAt(ihit));
          
	      hit->setID(detIDraw);
	      hit->setPE(cellPEs[detIDraw]);
	      hit->setMinPE(cellMinPEs[detIDraw]);
	      hit->setAmplitude(cellPEs[detIDraw]);
	      hit->setEnergy(depEnergy);
	      hit->setTime(Time[detIDraw]);
	      hit->setXpos(Xpos[detIDraw]); 
	      hit->setYpos(Ypos[detIDraw]); 
	      hit->setZpos(Zpos[detIDraw]);
	      hit->setNoise(false);
	      ihit++;

	      // checking fraction of edep that comes from primary beam electrons
	      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	    }

            if (verbose_) {
                detID_->setRawValue(detIDraw);
                detID_->unpack();
                int layer = detID_->getFieldValue("layer");
                int subsection = detID_->getFieldValue("section");
                int strip = detID_->getFieldValue("strip");

                std::cout << "detID: " << detIDraw << std::endl;
                std::cout << "Layer: " << layer << std::endl;
                std::cout << "Subsection: " << subsection << std::endl;
                std::cout << "Strip: " << strip << std::endl;
                std::cout << "Edep: " << Edep[detIDraw] << std::endl;
                std::cout << "numPEs: " << cellPEs[detIDraw] << std::endl;
                std::cout << "time: " << Time[detIDraw] << std::endl;
                std::cout << "z: " << Zpos[detIDraw] << std::endl;
                std::cout << "Layer: " << layer << "\t Strip: " << strip << "\t X: " << Xpos[detIDraw] <<  "\t Y: " << Ypos[detIDraw] <<  "\t Z: " << Zpos[detIDraw] << std::endl;
            }        // end verbose            
        }
        
        // ------------------------------- Noise simulation -------------------------------
	int numEmptyCells = NUM_STRIPS_PER_ARRAY_ - numRecHits; // only simulating for single array until all arrays are merged into one collection
	std::vector<double> noiseHits_PE = noiseGenerator_->generateNoiseHits( numEmptyCells );
	int detIDraw, tempID;
	
	for( int inoisehit = 0 ; inoisehit < noiseHits_PE.size() ; inoisehit++ ){
	  TrigScintHit *hit = (TrigScintHit*) (hits_->ConstructedAt(ihit));

	  // generate random ID from remoaining cells
	  // - - - - - -
	  detIDraw=2;  // for now subdet is alwways 2
	  do{
	    tempID = random_->Integer(NUM_STRIPS_PER_ARRAY_);
	  }while( Edep.find(tempID) != Edep.end() || 
		  noiseHitIDs.find(tempID) != noiseHitIDs.end() );
	  detIDraw ^= tempID<<4;

	  hit->setID(detIDraw);
	  hit->setPE(noiseHits_PE[ihit]);
	  hit->setMinPE(noiseHits_PE[ihit]);
	  hit->setAmplitude(noiseHits_PE[ihit]);
	  hit->setEnergy(0.);
	  hit->setTime(0.);
	  hit->setXpos(0.);
	  hit->setYpos(0.);
	  hit->setZpos(0.);
	  hit->setNoise(true);

	  ihit++;
	}
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        event.add(output_collection_, hits_);
    }

}

DECLARE_PRODUCER_NS(ldmx, TrigScintDigiProducer);

