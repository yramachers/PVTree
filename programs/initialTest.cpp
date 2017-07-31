#include "pvtree/treeSystem/treeFactory.hpp"
#include "pvtree/geometry/turtle.hpp"
#include <iostream>
#include <vector>
#include <memory>

int main() {
  auto ternaryTree = TreeFactory::instance()->getTree("ternary");

  // Change the parameters for the L-System to those chosen by student
  ternaryTree->setParameter("elongationRate", 1.109);
  ternaryTree->setParameter("widthIncreaseRate", 1.732);
  ternaryTree->setParameter("branchingAngle", 25.0);
  ternaryTree->setParameter("divergenceAngle1", 120.0);
  ternaryTree->setParameter("divergenceAngle2", 120.0);
  ternaryTree->setParameter("lengthScale", 50.0);

  // initialConditions
  std::vector<std::shared_ptr<TreeSystemInterface> > conditions =
      ternaryTree->getInitialConditions();

  ternaryTree->print();

  // Can go up to iterationNumber=12 within 4GB memory limit...
  unsigned int iterationNumber = 1;
  for (unsigned int i = 0; i < iterationNumber; i++) {
    std::vector<std::shared_ptr<TreeSystemInterface> > latestConditions;

    for (const auto& condition : conditions) {
      for (const auto& newCondition : condition->applyRule()) {
        latestConditions.push_back(newCondition);
      }
    }

    // Use the new iteration
    conditions.clear();
    conditions = latestConditions;

    // Display some information
    std::cout << "For iteration " << i << " there are " << conditions.size()
              << " conditions." << std::endl;
  }

  // Now create a set of turtles
  std::vector<Turtle*> activeTurtles;
  std::vector<Turtle*> retiredTurtles;

  // Create initial active turtle
  activeTurtles.push_back(new Turtle());

  // Process all the conditions (convert into turtles)
  int stepCount = 0;
  int printFrequency = 1;
  for (auto& condition : conditions) {
    condition->processTurtles(activeTurtles, retiredTurtles);

    if (stepCount % printFrequency == 0) {
      std::cout << "For step " << stepCount << " there are "
                << activeTurtles.size() << " active turtles and "
                << retiredTurtles.size() << " complete turtles." << std::endl;
    }

    stepCount++;
  }

  // Remove the last active turtle
  delete activeTurtles.back();
  activeTurtles.pop_back();

  std::cout << "For step " << stepCount << " there are " << activeTurtles.size()
            << " active turtles and " << retiredTurtles.size()
            << " complete turtles." << std::endl;

  // Iterate over all the turtles and print out the child numbers
  for (auto& turtle : retiredTurtles) {
    std::cout << "child number = " << turtle->children.size() << std::endl;
  }

  // Finally delete all the turtles
  for (auto& turtle : retiredTurtles) {
    delete turtle;
  }
  retiredTurtles.clear();
}
