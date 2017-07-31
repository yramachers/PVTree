/*!
 * @file
 * \brief Application to combine lightfields.
 *
 */
#include <iostream>
#include <string>

// ROOT includes
#include "TFile.h"
#include "TH2D.h"

// PVTree includes
#include "pvtree/full/solarSimulation/plenoptic3D.hpp"
#include "pvtree/utils/getopt_pp.h"

void showHelp() {
  std::cout << "lightfieldCombiner help" << std::endl;
  std::cout << "\t -i, --inputRootFiles <ROOT FILE NAMES>" << std::endl;
  std::cout << "\t -o, --outputRootFile <ROOT FILE NAME> :\t default "
               "combined.lightfield.root" << std::endl;
}

int main(int argc, char** argv) {
  std::vector<std::string> inputFilenames;
  std::string outputFilename;

  GetOpt::GetOpt_pp ops(argc, argv);

  // Check for help request
  if (ops >> GetOpt::OptionPresent('h', "help")) {
    showHelp();
    return 0;
  }

  ops >> GetOpt::Option('i', "inputRootFiles", inputFilenames);
  ops >> GetOpt::Option('o', "outputRootFile", outputFilename,
                        "combined.lightfield.root");

  if (inputFilenames.size() == 0) {
    std::cerr << "No input filenames specified" << std::endl;
    showHelp();
    return -1;
  }

  if (outputFilename == "") {
    std::cerr << "Empty output filename" << std::endl;
    showHelp();
    return -1;
  }

  // Also do not run if other arguments are present
  if (ops.options_remain()) {
    std::cerr << "Oops! Unexpected options." << std::endl;
    showHelp();
    return -1;
  }

  std::vector<Plenoptic3D*> lightfields;

  // Handle all the input root files
  for (unsigned int inputFileNumber = 0;
       inputFileNumber < inputFilenames.size(); inputFileNumber++) {
    if ((inputFileNumber) % 10 == 0) {
      std::cout << "Considering input file " << inputFileNumber << std::endl;
    }

    TFile currentInputFile(inputFilenames[inputFileNumber].c_str(), "READ");

    Plenoptic3D* currentLightfield =
        static_cast<Plenoptic3D*>(currentInputFile.Get("lightfield"));

    lightfields.push_back(
        static_cast<Plenoptic3D*>(currentLightfield->Clone()));

    currentInputFile.Close();
  }

  // Merge them all together
  for (unsigned int l = 1; l < lightfields.size(); l++) {
    lightfields[0]->append(*(lightfields[l]));
  }

  lightfields[0]->estimateSurfaceFluxes();

  // Prepare a root file to store the results
  TFile outputCombinedFile(outputFilename.c_str(), "RECREATE");

  lightfields[0]->Write("lightfield");
  lightfields[0]->getEnergyProjectedHistogram()->Write("projectedLightfield");

  // Close the root file
  outputCombinedFile.Close();
}
