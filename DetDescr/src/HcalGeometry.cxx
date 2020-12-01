#include "DetDescr/HcalGeometry.h"

#include <assert.h>
#include <iostream>
#include <iomanip>

namespace ldmx {

    HcalGeometry::HcalGeometry(const Parameters& ps) : ConditionsObject(HcalGeometry::CONDITIONS_OBJECT_NAME) {

        hcalThicknessScint_ = ps.getParameter<double>("hcalThicknessScint");
	hcalWidthScint_     = ps.getParameter<double>("hcalWidthScint");
        hcalZeroLayer_      = ps.getParameter<std::vector< double >>("hcalZeroLayer");
        hcalZeroStrip_      = ps.getParameter<std::vector< double >>("hcalZeroStrip");
        hcalLayerThickness_ = ps.getParameter<std::vector< double >>("hcalLayerThickness");
        hcalNLayers_        = ps.getParameter<std::vector< int >>("hcalNLayers");
        hcalNStrips_        = ps.getParameter<std::vector< int >>("hcalNStrips");
        verbose_            = ps.getParameter<int>("verbose");

	buildStripPositionMap();
    }

    void HcalGeometry::buildStripPositionMap() {

      for(int section=0; section<=HcalID::HcalSection::LEFT; section++) {
	HcalID::HcalSection hcalsection = (HcalID::HcalSection) section;
	for(int layer=0; layer<hcalNLayers_[section]; layer++) {
	  double layercenter = layer*hcalLayerThickness_.at( section ) + 0.5*hcalThicknessScint_;
	  for(int strip=0; strip<hcalNStrips_[section]; strip++) {
	    double stripcenter = (strip + 0.5)*hcalWidthScint_;
	    double x{-10000},y{-10000},z{-10000};

	    if(hcalsection == HcalID::HcalSection::BACK ) {
	      z = hcalZeroLayer_.at( section ) + layercenter;
	      if(layer % 2 == 1){ // odd layers have bars horizontal
		y = -1*hcalZeroStrip_.at( section ) + stripcenter;
	      }
	      else{ // even layers have bars vertical 
		x = -1*hcalZeroStrip_.at( section ) + stripcenter;
	      }
	    }
	    else{ 
	      z = hcalZeroStrip_.at( section ) + stripcenter;
	      if ( hcalsection == HcalID::HcalSection::TOP or hcalsection == HcalID::HcalSection::BOTTOM ) {
                y = hcalZeroLayer_.at( section ) + layercenter;
		//y = hcalZeroStrip_.at( section ) + stripcenter;
		// also v confused here
		if ( hcalsection == HcalID::HcalSection::BOTTOM ) {
		  y *= -1;
                }
	      }
	      else{
                x = hcalZeroLayer_.at( section ) + layercenter;
                //x = hcalZeroStrip_.at( section ) + stripcenter;
		// confused here too
		if ( hcalsection == HcalID::HcalSection::RIGHT ) {
		  x *= -1;
		}
	      }
	    }
	    stripPositionMap_[HcalID(section,layer,strip)] = std::tuple<double,double,double>(x,y,z);
	  } // loop over strips
	} // loop over layers
      } // loop over sections
    } // strip position map

}
