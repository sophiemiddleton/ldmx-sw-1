/**
 * @file HcalDigiID.h
 * @brief Class that defines an HCal digi detector ID
 * @author Cristina Suarez, Fermi National Accelerator Laboratory
 */

#ifndef DETDESCR_HCALDIGIID_H_
#define DETDESCR_HCALDIGIID_H_

// LDMX
#include "DetDescr/DetectorID.h"

namespace ldmx {

    /**
     * @class HcalDigiID
     * @brief Extension of DetectorID providing access to HCal digi information
     */
    class HcalDigiID : public DetectorID {	
	public:

	/**
	 * Encodes the section of the HCal based on the 'section' field value.
	 */
	enum HcalSection {
	    BACK = 0,
	    TOP = 1,
	    BOTTOM = 2,
	    LEFT = 4,
	    RIGHT = 3
	};
      
	static const RawValue SIDE_MASK{0x1}; // space for up to 2 ends of a strip
        static const RawValue SIDE_SHIFT{24};
	static const RawValue SECTION_MASK{0x7}; // space for up to 7 sections
	static const RawValue SECTION_SHIFT{18};
	static const RawValue LAYER_MASK{0xFF}; // space for up to 255 layers
	static const RawValue LAYER_SHIFT{10};
	static const RawValue STRIP_MASK{0xFF}; // space for 255 strips/layer
	static const RawValue STRIP_SHIFT{0};
	
	
	/**
	 * Empty HCAL id (but not null!)
	 */
        HcalDigiID() : DetectorID(SD_HCAL,0) { }
	
	/**
	 * Create from raw number
	 */
	HcalDigiID(RawValue rawid) : DetectorID(rawid) {
	    SUBDETECTORID_TEST("HcalDigiID", SD_HCAL);
	}

      	/**
	 * Create from a DetectorID, but check
	 */
	HcalDigiID(const DetectorID id) : DetectorID(id) {
  	    SUBDETECTORID_TEST("HcalDigiID", SD_HCAL);
	}

    
        /**
	 * Create from pieces
	 */
        HcalDigiID(unsigned int section, unsigned int layer, unsigned int strip, unsigned int side) : DetectorID(SD_HCAL,0) {
	    id_|=(section&SECTION_MASK)<<SECTION_SHIFT;
	    id_|=(layer&LAYER_MASK)<<LAYER_SHIFT;
	    id_|=(strip&STRIP_MASK)<<STRIP_SHIFT;
	    id_|=(side&SIDE_MASK)<<SIDE_SHIFT;
	}
	
	
	/**
	 * Get the value of the 'section' field from the ID.
	 * @return The value of the 'section' field.
	 */
	int getSection() const {
	    return (id_>>SECTION_SHIFT)&SECTION_MASK;
	}

        /**
	 * Get the value of the 'section' field from the ID.
	 * @return The value of the 'section' field.
	 */
	int section() const {
	    return (id_>>SECTION_SHIFT)&SECTION_MASK;
	}

 	/**
	 * Get the value of the layer field from the ID.
	 * @return The value of the layer field.
	 */
	int layer() const {
	    return (id_>>LAYER_SHIFT) & LAYER_MASK;
	}

	/**
	 * Get the value of the layer field from the ID.
	 * @return The value of the layer field.
	 */
	int getLayerID() const {
	    return (id_>>LAYER_SHIFT) & LAYER_MASK;
	}

        /**
	 * Get the value of the 'strip' field from the ID.
	 * @return The value of 'strip' field.
	 */
	int getStrip() const {
	    return (id_>>STRIP_SHIFT) & STRIP_MASK;
	}

	/**
	 * Get the value of the 'strip' field from the ID.
	 * @return The value of 'strip' field.
	 */
	int strip() const {
	    return (id_>>STRIP_SHIFT) & STRIP_MASK;
	}

	/**
         * Get the value of the 'side' field from the ID.
         * @return The value of the 'side' field.
         */
        int getSide() const {
	  return (id_>>SIDE_SHIFT)&SIDE_MASK;
        }

        /**                                                                                                                                                                      
	 * Get the value of the 'side' field from the ID.
         * @return The value of the 'side' field.
         */
        int side() const {
	  return (id_>>SIDE_SHIFT)&SIDE_MASK;
        }

	static void createInterpreters();
    };
}

std::ostream& operator<<(std::ostream&, const ldmx::HcalDigiID&);

#endif
