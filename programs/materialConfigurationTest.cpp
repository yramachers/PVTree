/*! @file
 * \brief Testing of material configuration via file.
 *
 */

#include "pvtree/full/material/materialFactory.hpp"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <vector>

int main() {
  // Set the default materials to be used
  MaterialFactory::instance()->addConfigurationFile("defaults-tree.cfg");

  // Get some materials
  MaterialFactory::instance()->getMaterial("pv-air");
  MaterialFactory::instance()->getMaterial("pv-aluminium");
  MaterialFactory::instance()->getMaterial("pv-concrete");
  MaterialFactory::instance()->getMaterial("pv-glass");
  MaterialFactory::instance()->getMaterial("pv-plastic");
  MaterialFactory::instance()->getMaterial("pv-silicon");

  return 0;
}

