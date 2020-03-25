/**
 * @file TrigScintDQM.cxx
 * @brief Analyzer used for TrigScint DQM. 
 * @author Omar Moreno, SLAC National Accelerator
 * @author Lene Kristian Bryngemark, Stanford University 
 */

#include "DQM/TrigScintDQM.h" 

namespace ldmx { 

    TrigScintDQM::TrigScintDQM(const std::string &name, Process &process) : 
        Analyzer(name, process) { }

    TrigScintDQM::~TrigScintDQM() {}

    void TrigScintDQM::onProcessStart() {

      std::cout << "Process starts!" << std::endl;
       
        // Get an instance of the histogram pool  
        histograms_ = HistogramPool::getInstance();    


        // Move into the TrigScint directory
	getHistoDirectory();

	//would like to avoid individual naming of histograms -- they are in different histogram dirs so i would have thought that they could be written individually. however i think this would require passing a name to getInstance() or similar. without that it looks like i always end up writing to the latest set of histograms created. 

	// so this is what i would like to, but can't do, for now:

	/*
	histDir_ = getHistoDirectory();

	histograms_->create<TH1F>("id", "Channel ID of sim hit", 100, 0, 100);
	histograms_->create<TH1F>("total_energy", "Total energy deposition in the pad/event", 3000, 0, 3000);
	histograms_->create<TH1F>("n_hits", "TrigScint hit multiplicity in the pad/event", 300, 0, 300);
	histograms_->create<TH1F>("x", "Hit x position", 1000, -100, 100);
	histograms_->create<TH1F>("y", "Hit y position", 1000, -100, 100);
	histograms_->create<TH1F>("z", "Hit z position", 1000, -900, 100);
	
	histograms_->create<TH1F>("energy", "Energy deposition in a TrigScint bar", 1500, 0, 1500);
	histograms_->create<TH1F>("hit_time", "TrigScint hit time (ns)", 1600, -100, 1500);
	
        histograms_->create<TH2F>("max_pe:time", 
                                  "Max Photoelectrons in a TrigScint bar", 1500, 0, 1500, 
                                  "TrigScint max PE hit time (ns)", 1500, 0, 1500);

        histograms_->create<TH2F>("min_time_hit_above_thresh:pe", 
                                  "Photoelectrons in a TrigScint bar", 1500, 0, 1500, 
                                  "Earliest time of TrigScint hit above threshold (ns)", 1600, -100, 1500);
*/

	// /*

	// instead do this 

	histograms_->create<TH1F>(Form("id_%s", padName_.c_str()), "Channel ID of sim hit", 100, 0, 100);
        histograms_->create<TH1F>(Form("total_energy_%s", padName_.c_str()), "Total energy deposition in the pad/event", 3000, 0, 3000);
        histograms_->create<TH1F>(Form("n_hits_%s", padName_.c_str()), "TrigScint hit multiplicity in the pad/event", 300, 0, 300);
        histograms_->create<TH1F>(Form("x_%s", padName_.c_str()), "Hit x position", 1000, -100, 100);
        histograms_->create<TH1F>(Form("y_%s", padName_.c_str()), "Hit y position", 1000, -100, 100);
        histograms_->create<TH1F>(Form("z_%s", padName_.c_str()), "Hit z position", 1000, -900, 100);

        histograms_->create<TH1F>(Form("energy_%s", padName_.c_str()), "Energy deposition in a TrigScint bar", 1500, 0, 1500);
        histograms_->create<TH1F>(Form("hit_time_%s", padName_.c_str()), "TrigScint hit time (ns)", 1600, -100, 1500);

        histograms_->create<TH2F>(Form("max_pe:time_%s", padName_.c_str()),
                                  "Max Photoelectrons in a TrigScint bar", 1500, 0, 1500,
                                  "TrigScint max PE hit time (ns)", 1500, 0, 1500);

        histograms_->create<TH2F>(Form("min_time_hit_above_thresh:pe_%s", padName_.c_str()),
                                  "Photoelectrons in a TrigScint bar", 1500, 0, 1500,
                                  "Earliest time of TrigScint hit above threshold (ns)", 1600, -100, 1500);


				  // */


	// TODO: implement getting a list of the constructed histograms, to set overflow boolean. 


    }

    void TrigScintDQM::configure(const ParameterSet& ps) {
      hitCollectionName_ = ps.getString("hit_collection");
      padName_ = ps.getString("pad").c_str();

      std::cout << "In configure, got parameters " << hitCollectionName_ << " and " << padName_ << std::endl;

    }

    void TrigScintDQM::analyze(const Event & event) { 


        // Check if the collection of digitized TrigScint hits exist. If it doesn't 
        // don't continue processing.
      if ( !event.exists(hitCollectionName_.c_str()) ) {
	std::cout << "No collection called " << hitCollectionName_ << std::endl;
	return; 
      }

      // Get the collection of TrigScintDQM digitized hits if the exists 
      //        const std::vector<TrigScintHit> TrigScintHits = event.getCollection<TrigScintHit>( hitCollectionName_);

      // trigger scintillator digi not in yet. this is the sim hits DQM. 
      // TODO implement DQM for digi. mostly same things but maxPE etc
      
      const std::vector<SimCalorimeterHit> TrigScintHits = event.getCollection<SimCalorimeterHit>( hitCollectionName_);
     
	const char * pName = padName_.c_str();

        // Get the total hit count
        int hitCount = TrigScintHits.size();  
        histograms_->get(Form("n_hits_%s", pName))->Fill(hitCount); 

        double totalEnergy{0};  

        // Loop through all TrigScint hits in the event
        // Get non-noise generated hits into new vector for sorting

	//skip for now, this is sim hits
	//        std::vector<const TrigScintHit *> filteredHits;
	//        for (const TrigScintHit &hit : TrigScintHits ) {
        for (const SimCalorimeterHit &hit : TrigScintHits ) {

	  histograms_->get(Form("energy_%s", pName))->Fill(hit.getEdep());  //(add histo for) pe for digi
	  histograms_->get(Form("hit_time_%s", pName))->Fill(hit.getTime());
	  histograms_->get(Form("id_%s", pName))->Fill(hit.getID()>>4 );

	  std::vector<float> posvec = hit.getPosition();
	  histograms_->get(Form("x_%s", pName))->Fill( posvec.at(0) );
	  histograms_->get(Form("y_%s", pName))->Fill( posvec.at(1) );
	  histograms_->get(Form("z_%s", pName))->Fill( posvec.at(2) );
           
	  totalEnergy += hit.getEdep();  // pe for digi

	  //            if ( hit.getTime() > -999. ) { filteredHits.push_back( &hit ); }
        }
        
	//        histograms_->get(Form("total_pe_%s", pName))->Fill(totalPE); 
        histograms_->get(Form("total_energy_%s", pName))->Fill(totalEnergy); 

	/* from hcal dqm, keep for later, tweak for trigscint, only for digi

        // Sort the array by hit time
        std::sort (filteredHits.begin(), filteredHits.end(), [ ](const auto& lhs, const auto& rhs) 
        {
            return lhs->getTime() < rhs->getTime(); 
        });
        
        //get first time and PE of hit over threshold
        double minTime{-1}; 
        double minTimePE{-1}; 
        for (const auto& hit : filteredHits) { 
            if (hit->getPE() < maxPEThreshold_) continue; 
            minTime = hit->getTime(); 
            minTimePE = hit->getPE(); 
            break;
        } 

        histograms_->get(Form("min_time_hit_above_thresh_%s", pName))->Fill(minTime); 
        histograms_->get(Form("min_time_hit_above_thresh:pe_%s", pName))->Fill(minTimePE, minTime);  

	*/

    }

} // ldmx

DECLARE_ANALYZER_NS(ldmx, TrigScintDQM)
