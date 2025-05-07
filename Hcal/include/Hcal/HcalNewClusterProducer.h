/**
 * @file HcalNewClusterProducer.h
 * @brief Class that performs clustering of HCal hits
 * @author Sophie Middleton, Caltech
 */

#ifndef HCAL_HcalNewClusterProducer_H_
#define HCAL_HcalNewClusterProducer_H_

// ROOT
#include "TRandom3.h"
#include "TString.h"

// LDMX
#include "DetDescr/DetectorID.h"
#include "DetDescr/HcalID.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "Tools/NoiseGenerator.h"

// Hcal
#include "DetDescr/HcalGeometry.h"
#include "Hcal/Event/HcalCluster.h"
#include "Hcal/Event/HcalHit.h"
#include "Hcal/MyClusterWeight.h"
#include "Hcal/TemplatedClusterFinder.h"
#include "Hcal/WorkingCluster.h"
namespace hcal {

/**
 * @class HcalNewClusterProducer
 * @brief Make clusters from hits in the HCAL
 */
class HcalNewClusterProducer : public framework::Producer {
 public:
  HcalNewClusterProducer(const std::string& name, framework::Process& process);

  virtual ~HcalNewClusterProducer() { ; }

  /**
   * Configure the processor using the given user specified parameters.
   *
   * @param parameters Set of parameters used to configure this processor.
   */
  void configure(framework::config::Parameters& parameters) final override;

  virtual void produce(framework::Event& event);

  TVector3 getPosition(std::map<ldmx::HcalID, TVector3> positionMap, ldmx::HcalID thisHit);

  void AddBackStripNeighbor(int id1, int id2);
  void AddBackLayerNeighbor(int id1, int id2);

  void MakeBackNeighborMap(std::map<ldmx::HcalID, TVector3> positionMap);

  void make2DClusters(framework::Event& event, std::vector<ldmx::HcalCluster>& hcalClusters);

  void fillClusters(const std::vector<ldmx::HcalHit>& hcalHits, const std::vector<int>& clusterList, std::vector<ldmx::HcalCluster>& hcalClusters);

  std::vector<ldmx::HcalCluster> finalCluster(std::vector<ldmx::HcalCluster>& hcalClusterList);
  std::map<int, std::vector<int> > back_strip_neighbors;
  std::map<int, std::vector<int> > back_layer_neighbors;

 private:
  bool verbose_{false};
  double EminSeed_{0.};
  double EnoiseCut_{0.};
  double deltaTime_{0};
  double deltaR_{0};
  double deltaZ_{0};
  double EminCluster_{0.};
  double cutOff_{0.};
  std::string clusterCollName_;
};

}  // namespace hcal

#endif
