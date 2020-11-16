#include "DetDescr/HcalReadout.h"

#include <assert.h>
#include <iostream>
#include <iomanip>

namespace ldmx {

    HcalReadout::HcalReadout(const Parameters& ps) : ConditionsObject(HcalReadout::CONDITIONS_OBJECT_NAME) {

        hcalThicknessScint_ = ps.getParameter<double>("hcalThicknessScint");
	hcalWidthScint_     = ps.getParameter<double>("hcalWidthScint");
        // hcalZeroLayer_      = ps.getParameter<std::map< HcalID::HcalSection , double >>("hcalZeroLayer");
	// hcalZeroStrip_      = ps.getParameter<std::map< HcalID::HcalSection , double >>("hcalZeroStrip");
        // hcalLayerThickness_ = ps.getParameter<std::map< HcalID::HcalSection , double >>("hcalLayerThickness");
	// hcalNLayers_        = ps.getParameter<std::map< HcalID::HcalSection , int >>("hcalNLayers");
        // hcalNStrips_        = ps.getParameter<std::map< HcalID::HcalSection , int >>("hcalNStrips");
        hcalZeroLayer_      = ps.getParameter<std::vector< double >>("hcalZeroLayer");
        hcalZeroStrip_      = ps.getParameter<std::vector< double >>("hcalZeroStrip");
        hcalLayerThickness_ = ps.getParameter<std::vector< double >>("hcalLayerThickness");
        hcalNLayers_        = ps.getParameter<std::vector< int >>("hcalNLayers");
        hcalNStrips_        = ps.getParameter<std::vector< int >>("hcalNStrips");
        verbose_            = ps.getParameter<int>("verbose");

	buildStripPositionMap();
    }

    void HcalReadout::buildStripPositionMap() {
      double x{0},y{0};
      for(int section=0; section<=HcalID::HcalSection::LEFT; section++) {
	HcalID::HcalSection hcalsection = (HcalID::HcalSection) section;
	for(int layer=0; layer<hcalNLayers_[section]; layer++) {
	  for(int strip=0; strip<hcalNStrips_[section]; strip++) {
	    double stripcenter = (strip + 0.5)*hcalWidthScint_;

	    if(hcalsection == HcalID::HcalSection::BACK ) {
	      if(layer % 2 == 1){ // odd layers have bars horizontal
		y = -1*hcalZeroStrip_.at( section ) + stripcenter;
	      }
	      else{ // even layers have bars vertical 
		x = -hcalZeroStrip_.at( section ) + stripcenter;
	      }
	    }
	    else{ 
	      if ( hcalsection == HcalID::HcalSection::TOP or hcalsection == HcalID::HcalSection::BOTTOM ) {
		y = hcalZeroStrip_.at( section ) + stripcenter;
		// also v confused here
		if ( hcalsection == HcalID::HcalSection::BOTTOM ) {
		  y *= -1;
                }
	      }
	      else{
                x = hcalZeroStrip_.at( section ) + stripcenter;
		// confused here too
		if ( hcalsection == HcalID::HcalSection::RIGHT ) {
		  x *= -1;
		}
	      }
	    }
	    stripPositionMap_[HcalID(section,layer,strip)] = std::pair<double,double>(x,y);
	  } // loop over strips
	} // loop over layers
      } // loop over sections
    } // strip position map

}
