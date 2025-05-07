#include "SimCore/LHE/LHEReader.h"

// STL
#include <iostream>
#include "TLorentzVector.h"
namespace simcore::lhe {

LHEReader::LHEReader(std::string& filename) {
  std::cout << "Opening LHE file " << filename << std::endl;
  ifs_.open(filename.c_str(), std::ifstream::in);
}

LHEReader::~LHEReader() { ifs_.close(); }

LHEEvent* LHEReader::readNextEvent() {
  std::string line;
  bool foundEventElement = false;
  while (getline(ifs_, line)) {
    if (line == "<event>") {
      foundEventElement = true;
      break;
    }
  }

  if (!foundEventElement) {
    std::cerr << "WARNING: No next <event> element was found by the LHE reader."
              << std::endl;
    return nullptr;
  }

  getline(ifs_, line);

  LHEEvent* nextEvent = new LHEEvent(line);

  while (getline(ifs_, line)) {
    if (line == "</event>" || line == "<mgrwt>") {
      // break if the event ended or in LHE 3.0 if we reach the mgrwt block
      break;
    }
    int proc = 1; //1=prima, 2=PF
    if (line.find("#") == std::string::npos) {  // not a comment line
      LHEParticle* particle = new LHEParticle(line);
      if(proc == 2 and particle->getIDUP() == 666){
        double alp_vtim = particle->getVTIMUP();
        double alp_mass=particle->getPUP(4);
        double alp_px=particle->getPUP(0);
        double alp_py=particle->getPUP(1);
        double alp_pz=particle->getPUP(2);
        TLorentzVector p4 = TLorentzVector(particle->getPUP(0),particle->getPUP(1),particle->getPUP(2),particle->getPUP(3));
        double alp_gamma=p4.Gamma();
        nextEvent->setVertex(alp_mass,alp_px,alp_py,alp_pz,alp_vtim,alp_gamma);
      }
      nextEvent->addParticle(particle);
    } else {
      if (line.find("#vertex") != std::string::npos) {
        nextEvent->setVertex(line);
      }
    }
  }

  const std::vector<LHEParticle*>& particles = nextEvent->getParticles();
  int particleIndex = 0;
  for (std::vector<LHEParticle*>::const_iterator it = particles.begin();
       it != particles.end(); it++) {
    LHEParticle* particle = (*it);
    if (particle->getMOTHUP(0) != 0) {
      int mother1 = particle->getMOTHUP(0);
      int mother2 = particle->getMOTHUP(1);
      if (mother1 > 0) {
        particle->setMother(0, particles[mother1 - 1]);
      }
      if (mother2 > 0) {
        particle->setMother(1, particles[mother2 - 1]);
      }
    }
    ++particleIndex;
  }

  return nextEvent;
}

}  // namespace simcore::lhe
