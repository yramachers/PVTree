#include "pvtree/leafSystem/leafFactory.hpp"
#include "pvtree/geometry/turtle.hpp"
#include "pvtree/utils/getopt_pp.h"
#include <iostream>
#include <vector>
#include <memory>

void showHelp() {
  std::cout << "leafCheck help" << std::endl;
  std::cout << "\t -l, --leaf <LEAF TYPE NAME>" << std::endl;
  std::cout << "\t -i, --iterationNumber <INTEGER>" << std::endl;
}

int main(int argc, char** argv) {

  std::string leafType;
  unsigned int iterationNumber;
  GetOpt::GetOpt_pp ops(argc, argv);

  // Check for help request
  if (ops >> GetOpt::OptionPresent('h', "help")){
    showHelp();
    return 0;
  }

  ops >> GetOpt::Option('l', "leaf", leafType, "rose");
  ops >> GetOpt::Option('i', "iterationNumber", iterationNumber, 5u);

  // Also do not run if other arguments are present
  if (ops.options_remain()) {
    std::cerr << "Oops! Unexpected options." << std::endl;
    showHelp();
    return -1;
  }

  auto leaf = LeafFactory::instance()->getLeaf(leafType);

  //initialConditions
  std::vector<std::shared_ptr<LeafSystemInterface> > conditions = leaf->getInitialConditions();
  leaf->print();

  for (unsigned int i=0; i<iterationNumber; i++){
    std::vector<std::shared_ptr<LeafSystemInterface> > latestConditions;

    for (const auto & condition : conditions) {
      for (const auto & newCondition : condition->applyRule()) {
	latestConditions.push_back(newCondition);
      }
    }
    
    //Use the new iteration
    conditions.clear();
    conditions = latestConditions;

    //Display some information
    std::cout << "For iteration " << i << " there are " << conditions.size() << " conditions." << std::endl;

    std::cout << "Produced Rules = ";
    for (unsigned int x=0; x<conditions.size(); x++) {
      conditions[x]->print(std::cout);
    }
    std::cout << std::endl;
  }
}






