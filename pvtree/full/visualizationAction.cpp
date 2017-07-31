#include "pvtree/full/visualizationAction.hpp"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4VVisManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4Box.hh"

VisualizationAction::VisualizationAction(G4LogicalVolume* volumeToDraw)
    : m_volumeToDraw(volumeToDraw),
      m_defaultVisualAttributes(G4Colour(1, 1, 0)) {}

void VisualizationAction::Draw() {
  G4VVisManager* pVisManager = G4VVisManager::GetConcreteInstance();

  if (pVisManager) {
    // Start by drawing the world
    pVisManager->Draw(*m_volumeToDraw, *(m_volumeToDraw->GetVisAttributes()),
                      G4Transform3D());

    // Recursively draw all children
    for (int c = 0; c < m_volumeToDraw->GetNoDaughters(); c++) {
      DrawRecursively(pVisManager, m_volumeToDraw->GetDaughter(c));
    }
  }
}

void VisualizationAction::DrawRecursively(
    G4VVisManager* visManager, G4VPhysicalVolume* currentPhysicalVolume) {
  // Draw current volume
  G4Transform3D currentTransform(
      currentPhysicalVolume->GetObjectRotationValue(),
      currentPhysicalVolume->GetObjectTranslation());

  const G4VisAttributes* currentVisualAttributes =
      currentPhysicalVolume->GetLogicalVolume()->GetVisAttributes();

  if (currentVisualAttributes != NULL) {
    if (currentVisualAttributes->IsVisible()) {
      visManager->Draw(*currentPhysicalVolume, *currentVisualAttributes,
                       currentTransform);
    }

  } else {
    // No visual attributes, draw with default attributes
    visManager->Draw(*currentPhysicalVolume, m_defaultVisualAttributes,
                     currentTransform);
  }

  // Also draw any children
  G4LogicalVolume* currentLogicalVolume =
      currentPhysicalVolume->GetLogicalVolume();

  for (int c = 0; c < currentLogicalVolume->GetNoDaughters(); c++) {
    DrawRecursively(visManager, currentLogicalVolume->GetDaughter(c));
  }
}
