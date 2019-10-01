# declare SimPlugins module
module(
  NAME SimPlugins
  DEPENDENCIES SimCore 
  EXTERNAL_DEPENDENCIES Geant4 ROOT gsl
  EXTRA_LINK_LIBRARIES ${CMAKE_DL_LIBS} 
)
