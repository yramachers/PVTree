/*!
 * @file 
 * \brief Application to combine the results of the tree scan example.
 *
 * This method will probably break down at large-ish tree numbers, so
 * moving towards actual ntuples would perhaps be a good idea. Or say
 * just keeping constructors in TTrees!
 */
#include <iostream>
#include <string>

// ROOT includes
#include "TFile.h"
#include "TList.h"

// PVTree includes
#include "pvtree/treeSystem/treeConstructionInterface.hpp"
#include "pvtree/leafSystem/leafConstructionInterface.hpp"
#include "pvtree/utils/getopt_pp.h"

void showHelp() {
  std::cout << "ntupleCombiner help" << std::endl;
  std::cout << "\t -i, --inputRootFiles <ROOT FILE NAMES>" << std::endl;
  std::cout << "\t -o, --outputRootFile <ROOT FILE NAME> :\t default combined.results.root" << std::endl;
  std::cout << "\t --listNames <Space seperated strings> :\t default testedStructures" << std::endl;
}

int main(int argc, char** argv) {
  std::vector<std::string> inputFilenames;
  std::string outputFilename;
  std::vector<std::string> listNames;

  GetOpt::GetOpt_pp ops(argc, argv);

  // Check for help request
  if (ops >> GetOpt::OptionPresent('h', "help")){
    showHelp();
    return 0;
  }

  ops >> GetOpt::Option('i', "inputRootFiles", inputFilenames);
  ops >> GetOpt::Option('o', "outputRootFile", outputFilename, "combined.results.root");
  ops >> GetOpt::Option("listNames", listNames, {"testedStructures"});

  if (inputFilenames.size() == 0){
    std::cerr << "No input filenames specified" << std::endl;
    showHelp();
    return -1;
  }

  if (outputFilename == ""){
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

  // Fill a set of lists
  std::vector<TList*> listOfLists;
  for (unsigned int l=0; l<listNames.size(); l++){
    listOfLists.push_back(new TList);
  }

  // Handle all the input root files
  for (unsigned int inputFileNumber=0; inputFileNumber<inputFilenames.size(); inputFileNumber++) {

    if ((inputFileNumber) % 10 == 0) {
      std::cout << "Considering input file " << inputFileNumber << std::endl;
    }

    TFile currentInputFile(inputFilenames[inputFileNumber].c_str(), "READ");

    for (unsigned int l=0; l<listNames.size(); l++){
      TList* currentList = (TList*)currentInputFile.Get(listNames[l].c_str());

      for (int x=0; x<currentList->GetSize(); x++) {
	listOfLists[l]->Add(currentList->At(x)->Clone());
      }
    }

    currentInputFile.Close();
  }


  // Prepare a root file to store the results
  TFile outputCombinedFile(outputFilename.c_str(), "RECREATE");

  for (unsigned int l=0; l<listNames.size(); l++){
    listOfLists[l]->Write(listNames[l].c_str(), TObject::kSingleKey);
  }

  // Close the root file
  outputCombinedFile.Close();

  // Delete the lists
  for (unsigned int l=0; l<listNames.size(); l++){
    delete listOfLists[l];
  }
  listOfLists.clear();
}

