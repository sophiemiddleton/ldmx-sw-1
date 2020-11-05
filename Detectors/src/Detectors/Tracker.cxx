
#include "XML/Helper.h"
//#include "XML/XML.h"
#include "Acts/Plugins/DD4hep/ActsExtension.hpp"

using namespace dd4hep;

static Ref_t create_tracker(Detector &lcdd, xml::Handle_t xml_handle,
                            SensitiveDetector sens_det) {
    

  // Detector xml handle
  xml::DetElement det_handle = xml_handle;

  // Get the dimensions of the tracker envelope and construct a box shape made
  // out of air.  This volume will be used to contain the tracker.
  auto env_dims(det_handle.dimensions());
  Box env_box(env_dims.dx(), env_dims.dy(), env_dims.dz());
  Volume env_vol(det_handle.nameStr() + "_envelope", env_box, lcdd.air());
  // Set the attributes of the envelope
  env_vol.setAttributes(lcdd, det_handle.regionStr(), det_handle.limitsStr(),
                        det_handle.visStr());


  // Create the tracker detector element
  DetElement tracker(det_handle.nameStr(), det_handle.id());
  std::cout<<"PF::DEBUG "<<__PRETTY_FUNCTION__<<" name:"<<tracker.name()<< " type:"<<tracker.type()<<std::endl;
  
  //Add the ActsExtension for the Reco Geometry
  Acts::ActsExtension* trackerExtension = new Acts::ActsExtension();

  //Is name correct here? I think should be type() but returns empty above?
  trackerExtension->addType(tracker.name(), "detector");
  tracker.addExtension<Acts::ActsExtension>(trackerExtension);
  
  // Get the global position of the tracker envelope and place it in the mother
  // volume.
  auto env_pos{det_handle.position()};
  auto env_placed_vol(lcdd.pickMotherVolume(tracker).placeVolume(
      env_vol, Position(env_pos.x(), env_pos.y(), env_pos.z())));
  tracker.setPlacement(env_placed_vol);
  
  


  // The placed volume
  PlacedVolume pv;

  // Loop over all of the modules and create the sensor volumes.
  for (xml::Collection_t imodule(det_handle, _U(module)); imodule; ++imodule) {
    xml::Component xml_module(imodule);
    
    std::string moduleName = "module_" + std::to_string(xml_module.id());
    unsigned int moduleNum = xml_module.id();
    
    // The module detector element
    // A module is made out of 2 layers.
    DetElement moduleElement(moduleName+"_elementTemplate", 0);
    
    // Start by creating an assembly for the layers. An assembly will act as
    // bounding box for the two silicon layers it encloses.
    Assembly module_assembly(moduleName+"_assembly");
    
    //Visualization attributes -- empty for the moment.
    module_assembly.setVisAttributes(lcdd,xml_module.visStr());
    
    //Components
    unsigned int compNum = 0;
    
    //Sensors 
    unsigned int layerNum = 0;

    // Build up the layers inside of the assembly
    for (xml::Collection_t ilayer(xml_module, _U(layer)); ilayer; ++ilayer,++layerNum) {

      xml::Component xml_layer(ilayer);

      

      // Create the box shape representing the sensor.  If a box can't be
      // created, throw an exception.
      Box sensor_box{xml_layer.createShape()};
      if (!sensor_box.isValid()) {
        // throw an exception
        std::cout << "Cannot crate box volume." << std::endl;
      }

      // Create a volume out of the box and set the material it's made from.
      auto sensor_mat{lcdd.material(xml_layer.materialStr())};
      auto position{xml_layer.position()};
      std::string layerName= "module_" + std::to_string(xml_module.id()) + "_layer_" + std::to_string(xml_layer.id());
      unsigned int layerNum = xml_module.id()*10+xml_layer.id();
      Volume layer_vol(moduleName,sensor_box, sensor_mat);
      
      //Add the vis attributes?
      layer_vol.setVisAttributes(lcdd, xml_layer.visStr());

      // Rotate the sensor if a rotation was specified.
      RotationZYX rotation;
      if (xml_layer.hasChild(_U(rotation))) {
        rotation =
            RotationZYX(xml_layer.rotation().x(), xml_layer.rotation().y(),
                        xml_layer.rotation().z());
      } else
        rotation = RotationZYX(0., 0., 0.);

      // Place the sensor inside of the module assembly
      pv = module_assembly.placeVolume(
          layer_vol, Transform3D(rotation, Position(position.x(), position.y(),
                                                    position.z())));
      
      DetElement layerElement(moduleElement, layerName, layerNum);
      // Add the sensor extension
      Acts::ActsExtension* layerExtension = new Acts::ActsExtension();
      layerExtension->addType("layer", "detector");
      layerExtension->addType("axes", "definitions", "XYZ");
      layerElement.addExtension<Acts::ActsExtension>(layerExtension);
      
    }// loop over the layers

    // Get the position of the module and place it inside of the tracker
    // envelope.
    auto module_position{xml_module.position()};
    pv = env_vol.placeVolume(module_assembly,
                             Position(module_position.x(), module_position.y(),
                                      module_position.z()));

    // Clone the module detector element  (is this necessary?)
    auto moduleElement_clone = moduleElement.clone(moduleName, moduleNum);
    
    //add the module to the tracker
    tracker.add(moduleElement_clone);
    
  }// loop over the modules

  std::cout<<"PF::DEBUG "<<__PRETTY_FUNCTION__<<" Returning tracker"<<std::endl;
  return tracker;
}

DECLARE_DETELEMENT(Tracker, create_tracker)
