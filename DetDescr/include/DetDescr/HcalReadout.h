/**
 * @file EcalHexReadout.h
 * @brief Class that translates raw positions of HCal bar hits into cells in a hexagonal readout
 */

#ifndef DETDESCR_HCALREADOUT_H_
#define DETDESCR_HCALREADOUT_H_

// LDMX
#include "Framework/Exception.h"
#include "DetDescr/HcalID.h"
#include "Framework/Parameters.h"
#include "Framework/ConditionsObject.h"

// STL
#include <map>

namespace ldmx {

    class HcalGeometryProvider;
  
    /**
     * @class HcalReadout
     * @brief Implementation of HCal bar readout
     *
     */
    class HcalReadout : public ConditionsObject {

        public:
            static constexpr const char* CONDITIONS_OBJECT_NAME{"HcalReadout"};

            /**
             * Class destructor.
             *
             * Does nothing because the stl containers clean up automatically.
             */
            virtual ~HcalReadout() { }

            /**
             * Get entire real space position for the strip with the input raw ID
             *
             * Inputs x,y,z will be set to the calculated position
             *
             * @sa getStripCenterAbsolute and getZPosition
             *
             * @param[in] id HcalID for the strip we want the position of
             * @param[out] xy set to x/y-coordinate of strip center
             * @param[out] z set to z-coordinate of strip center
             */
            void getStripAbsolutePosition( HcalID id, double &x, double &y, double &z ) const {
	        std::pair<double,double> xy = this->getStripCenterAbsolute( id );
	        x = xy.first;
		y = xy.second;
                z = getZPosition(id);
                return;
            }

	    /**
	     * Get the z-coordinate given the layer id
	     *
             * @param[in] layer int layer id 
             * @return z-coordinate of the input sensitive layer
	     */
            double getZPosition(HcalID id) const {
	        int layer   = id.layer();
		HcalID::HcalSection hcalsection = (HcalID::HcalSection)id.section();
		int section = id.section();
	        double layercenter = layer*hcalLayerThickness_.at( section ) + 0.5*hcalThicknessScint_;
		double z =0;
		if(hcalsection == HcalID::HcalSection::BACK){
		  z = hcalZeroLayer_.at(section) + layercenter;
		}
		else{
		  z = hcalZeroStrip_.at(section) + layercenter; 
		  // need to x-check this because this is confusing https://github.com/LDMX-Software/ldmx-sw/blob/master/EventDisplay/src/DetectorGeometry.cxx#L264
		}
		return z;
            }

            /**
             * Get a strip center X and Y position relative to hcal center from a combined hcal ID
             *
             * @throw std::out_of_range if HcalID isn't created with valid bar or bar IDs.
             *
             * @param HcalID 
             * @return The X and Y position of the center of the bar.
             */
	    std::pair<double,double> getStripCenterAbsolute(HcalID id) const {
	      return stripPositionMap_.at(id);
            }

    static HcalReadout* debugMake(const Parameters& p) { return new HcalReadout(p); }
    
        private:

            /**
             * Class constructor, for use only by the provider
             *
             * @param ps Parameters to configure the HcalReadout
             */
            HcalReadout(const Parameters &ps);
            friend class HcalGeometryProvider;

            void buildStripPositionMap();

        private:

            /// verbosity, not configurable but helpful if developing
            int verbose_{2};

	    /// thickness of scintillator 
	    double hcalThicknessScint_{0.};

	    /// Width of Scintillator Strip [mm]
            double hcalWidthScint_{0.};

            /// Front of HCal relative to world geometry for each section [mm]
	    std::vector<double> hcalZeroLayer_;
	    //std::map< HcalID::HcalSection , double > hcalZeroLayer_;

            /// The plane of the zero'th strip of each section [mm] 
	    std::vector<double> hcalZeroStrip_;
	    //std::map< HcalID::HcalSection , double > hcalZeroStrip_;

	    /// Thickness of the layers in each section [mm] 
	    std::vector<double> hcalLayerThickness_;
	    //std::map< HcalID::HcalSection , double > hcalLayerThickness_;

	    /// Number of layers in each section 
	    std::vector<int> hcalNLayers_;
	    //std::map< HcalID::HcalSection , int > hcalNLayers_;

            /// Number of strips per layer in each section
	    std::vector<int> hcalNStrips_;
	    //std::map< HcalID::HcalSection , int > hcalNStrips_;

            /// Position of bar centers relative to world geometry (uses ID with real bar and section and layer as zero for key)
            std::map<HcalID, std::pair<double,double>> stripPositionMap_;
    };
    
}

#endif
