#include "SimCore/LHE/LHEEvent.h"

#include "Framework/Exception/Exception.h"

// Geant4
#include "globals.hh"

// STL
#include <iostream>
#include <sstream>
#include <random>
namespace simcore::lhe {

LHEEvent::LHEEvent(std::string& line) {
  std::istringstream iss(line);
  std::vector<std::string> tokens;
  do {
    std::string elem;
    iss >> elem;
    if (elem.size() != 0) {
      tokens.push_back(elem);
    }
  } while (iss);

  if (tokens.size() != 6) {
    EXCEPTION_RAISE("TokenNum",
                    "Wrong number of tokens in LHE event information record.");
  }

  nup_ = atoi(tokens[0].c_str());
  idprup_ = atoi(tokens[1].c_str());
  xwgtup_ = atof(tokens[2].c_str());
  scalup_ = atof(tokens[3].c_str());
  aqedup_ = atof(tokens[4].c_str());
  aqcdup_ = atof(tokens[5].c_str());

  vtx_[0] = 0;
  vtx_[1] = 0;
  vtx_[2] = 0;
}

LHEEvent::~LHEEvent() {
  for (std::vector<LHEParticle*>::iterator it = particles_.begin();
       it != particles_.end(); it++) {
    delete (*it);
  }
  particles_.clear();
}

int LHEEvent::getNUP() const { return nup_; }

int LHEEvent::getIDPRUP() const { return idprup_; }

double LHEEvent::getXWGTUP() const { return xwgtup_; }

double LHEEvent::getSCALUP() const { return scalup_; }

double LHEEvent::getAQEDUP() const { return aqedup_; }

double LHEEvent::getAQCDUP() const { return aqcdup_; }

const double* LHEEvent::getVertex() const { return vtx_; }

double LHEEvent::getVertexTime() const { return vtxt_; }

void LHEEvent::addParticle(LHEParticle* particle) {
  particles_.push_back(particle);
}

const std::vector<LHEParticle*>& LHEEvent::getParticles() { return particles_; }

void LHEEvent::setVertex(double x, double y, double z) {
  vtx_[0] = x;
  vtx_[1] = y;
  vtx_[2] = z;
}

/**
 * Parse the vertex from a line of the form "#vertex [x] [y] [z] [t]"
 * Where [t] is assumed zero if not specified
 */
 void LHEEvent::setVertex(const std::string& line) {
   std::istringstream iss(line);
   std::vector<std::string> tokens;
   do {
     std::string elem;
     iss >> elem;
     if (elem.size() != 0) {
       tokens.push_back(elem);
     }
   } while (iss);

   if (tokens.size() != 4 && tokens.size() != 5) {
     EXCEPTION_RAISE("TokenNum",
                     "Wrong number of tokens or format in LHE event vertex "
                     "information record.");
   }

   vtx_[0] = atof(tokens[1].c_str());
   vtx_[1] = atof(tokens[2].c_str());
   vtx_[2] = atof(tokens[3].c_str());

   if (atof(tokens[1].c_str()) > 1500) vtx_[0] = 1500;
   if (atof(tokens[2].c_str()) > 1500) vtx_[1] = 1500;
   if (atof(tokens[1].c_str()) <  -1500) vtx_[0] = -1500;
   if (atof(tokens[2].c_str()) < -1500) vtx_[1] = -1500;
   if (atof(tokens[3].c_str()) > 5700) vtx_[2] = 5700;
   if (atof(tokens[3].c_str()) < 0) vtx_[2] = 0;
   if (tokens.size() > 4) {
     vtxt_ = atof(tokens[4].c_str());
   }
 }

void LHEEvent::setVertex(double alp_mass, double alp_px , double alp_py, double alp_pz, double alp_vtim, double gamma) {

  //double scale_x = (alp_px/(gamma*alp_mass))*alp_vtim;
  //double scale_y = (alp_py/(gamma*alp_mass))*alp_vtim;
  //double scale_z = (alp_pz/(gamma*alp_mass))*alp_vtim;

  //double coupling_scale = (1e-3 / 5e-5)*(1e-3 / 5e-5);
  //vtx_[0] = coupling_scale*scale_x;
  //vtx_[1] = coupling_scale*scale_y;
  //vtx_[2] = coupling_scale*scale_z;
  //if(uniform){
  // const int range_from  = 750.0;
//   const int range_to    = 5500.0;
// std::random_device                  rand_dev;
//   std::mt19937                        generator(rand_dev());
//   std::uniform_int_distribution<float>  distr(range_from, range_to);
//   float d = distr(generator);
//   float mag = sqrt(alp_px*alp_px + alp_py*alp_py + alp_pz*alp_pz);
//   vtx_[0] = 0 + mag * d;
//   vtx_[1] = 0 + mag * d;
//   vtx_[2] = 0 + mag * d;
  //}
  std::cout<<"vertex after"<<vtx_[0] <<","<<vtx_[1] <<","<<vtx_[2] <<std::endl;
  if (vtx_[0] > 1500) vtx_[0] = 1500;
  if (vtx_[1] > 1500) vtx_[1] = 1500;
  if (vtx_[1] <  -1500) vtx_[1] = -1500;
  if (vtx_[1] < -1500) vtx_[1] = -1500;
  if (vtx_[2]  > 5700) vtx_[2] = 5700;
  if (vtx_[2] < 0) vtx_[2] = 0;

}

}  // namespace simcore::lhe
