#ifndef PV_ROOT_GEOMETRY
#define PV_ROOT_GEOMETRY

#include <vector>
#include <string>
#include "TVector3.h"
#include "pvtree/rootBasedSimulation/leaf.hpp"

// Forward declare
class TGeoManager;
class TGeoVolume;
class TGeoMedium;
class Turtle;
class Sun;
class TH1D;

class ROOTGeometry {
 private:
  TGeoManager* m_manager;
  TGeoVolume* top;
  TGeoMedium* vacuumMedium;
  TGeoMedium* branchMedium;
  TGeoMedium* leafMedium;
  int volumeCount;
  bool boundingBoxesVisible;
  int depthStepNumber;

  std::vector<Leaf>
      leaves;  // Mainly for keeping track of simulation information.

  void buildLists(Turtle* turtle, std::vector<Turtle*>& toDraw,
                  std::vector<Turtle*>& toSeed, int depthCount);
  void getTurtleTreeExtent(const Turtle* turtle, TVector3& minExtent,
                           TVector3& maxExtent, int maxDepth = -1);
  void normalizeTurtlesHeight(std::vector<Turtle*> turtles, double height);
  void recursiveTreeBuild(Turtle* startTurtle, int depthStep,
                          TGeoVolume* parentVolume, TVector3 parentPosition);
  void constructLeaf(const Turtle* endTurtle, TGeoVolume* parentVolume,
                     TVector3 parentPosition);

 public:
  explicit ROOTGeometry(TGeoManager* manager);
  void constructTreeFromTurtles(std::vector<Turtle*> turtles, double maxZ);
  void close();
  void draw(std::string options = "");
  void evaluateEnergyCollection(Sun& sun);

  void setBoundingBoxVisibility(bool isVisible) {
    this->boundingBoxesVisible = isVisible;
  }
  bool getBoundingBoxVisibility() { return this->boundingBoxesVisible; }

  TGeoManager* getManager();
  std::vector<Leaf>& getLeaves();
};

#endif  // PV_ROOT_GEOMETRY
