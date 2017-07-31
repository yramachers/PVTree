#ifndef PVTREE_COMMAND_RANDOMIZE_DETECTOR_HPP
#define PVTREE_COMMAND_RANDOMIZE_DETECTOR_HPP

/*! \file
 * \brief Test out the idea of augmenting the Geant4 commands.
 */

#include "G4UImessenger.hh"

class DetectorConstruction;
class TreeConstructionInterface;
class LeafConstructionInterface;

class CommandRandomizeDetector : public G4UImessenger {
 public:
  CommandRandomizeDetector();
  virtual ~CommandRandomizeDetector();

 private:
  // Elements I will need to touch
  DetectorConstruction* m_detectorConstruction;
  TreeConstructionInterface* m_treeConstructionInterface;
  LeafConstructionInterface* m_leafConstructionInterface;
};

#endif  // PVTREE_COMMAND_RANDOMIZE_DETECTOR_HPP
