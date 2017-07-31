/*!
 * @file
 * \brief Application to combine the results of the convergence example.
 *
 * Just want to merge a subset of the histograms.
 */
#include <iostream>
#include <string>

// ROOT includes
#include "TFile.h"
#include "TH1D.h"
#include "TList.h"
#include "TKey.h"

// PVTree includes
#include "pvtree/utils/getopt_pp.h"

void showHelp() {
  std::cout << "convergenceCombiner help" << std::endl;
  std::cout << "\t -i, --inputRootFiles <ROOT FILE NAMES>" << std::endl;
  std::cout << "\t -o, --outputRootFile <ROOT FILE NAME>" << std::endl;
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
                        "combined.convergence.root");

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

  // Get the histograms/names to be merged from the first input file
  std::map<std::string, TH1D*> mergingHistograms;

  TFile initialInputFile(inputFilenames[0].c_str(), "READ");
  TList* keyList = initialInputFile.GetListOfKeys();
  TIter iterateOverKeys(keyList);

  for (int k = 0; k < keyList->GetSize(); k++) {
    TKey* key = (TKey*)iterateOverKeys();

    // Pick out histograms with names containing 'relative'
    std::string keyName = key->GetName();

    if (keyName.find("relative") != std::string::npos) {
      std::string newName = keyName + "_merged";

      TH1D* histogram = (TH1D*)initialInputFile.Get(keyName.c_str());
      mergingHistograms[keyName] = (TH1D*)histogram->Clone(newName.c_str());
    }
  }

  // Merge the efficiency error and energy error histograms
  for (unsigned int inputFileNumber = 1;
       inputFileNumber < inputFilenames.size(); inputFileNumber++) {
    TFile inputFile(inputFilenames[inputFileNumber].c_str(), "READ");

    for (auto& toMerge : mergingHistograms) {
      TH1D* histogram = (TH1D*)inputFile.Get(toMerge.first.c_str());
      toMerge.second->Add(histogram);
    }

    inputFile.Close();
  }

  // Prepare a root file to store the results
  TFile outputCombinedFile(outputFilename.c_str(), "RECREATE");

  // Write the results out to the root file
  for (auto& merged : mergingHistograms) {
    merged.second->Write(merged.first.c_str());
  }

  // Close the root file
  outputCombinedFile.Close();

  initialInputFile.Close();
}
