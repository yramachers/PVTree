/*!
 * @file
 * \brief Application to test the collection efficiency of randomly generated
 *        trees over a period of one day.
 *
 */

#include "pvtree/treeSystem/treeFactory.hpp"
#include "pvtree/leafSystem/leafFactory.hpp"
#include "pvtree/full/detectorConstruction.hpp"
#include "pvtree/full/actionInitialization.hpp"
#include "pvtree/full/primaryGeneratorAction.hpp"
#include "pvtree/full/opticalPhysicsList.hpp"
#include "pvtree/full/recorders/convergenceRecorder.hpp"
#include "pvtree/full/solarSimulation/sun.hpp"
#include "pvtree/analysis/yearlyResult.hpp"
#include "pvtree/full/material/materialFactory.hpp"
#include "pvtree/utils/getopt_pp.h"
#include "pvtree/utils/resource.hpp"
#include "pvtree/climate/climateFactory.hpp"
#include "pvtree/location/locationDetails.hpp"
#include "pvtree/full/solarSimulation/spectrumFactory.hpp"
#include "pvtree/utils/signalReceiver.hpp"

#include <iostream>
#include <vector>
#include <memory>
#include <chrono>
#include <sstream>
#include <csignal>

#include "globals.hh"
#include "G4VisExecutive.hh"
#include "G4VisExtent.hh"
#include "G4UImanager.hh"
#include "G4UIExecutive.hh"
#include "Randomize.hh"
#include "G4RunManager.hh"
#include "G4StepLimiterPhysics.hh"

// save diagnostic state
#pragma GCC diagnostic push

// turn off the specific warning.
#pragma GCC diagnostic ignored "-Wshadow"

#include "TFile.h"
#include "TGraphAsymmErrors.h"
#include "TAxis.h"
#include "TCanvas.h"
#include "TH1F.h"

// turn the warnings back on
#pragma GCC diagnostic pop

void showHelp() {
  std::cout << "treeScan help" << std::endl;
  std::cout << "\t -t, --tree <TREE TYPE NAME>" << std::endl;
  std::cout << "\t -l, --leaf <LEAF TYPE NAME>" << std::endl;
  std::cout << "\t --treeNumber <INTEGER>" << std::endl;
  std::cout << "\t --timeSegments <INTEGER>" << std::endl;
  std::cout << "\t --photonNumber <INTEGER>" << std::endl;
  std::cout << "\t --geant4Seed <INTEGER>" << std::endl;
  std::cout << "\t --parameterSeedOffset <INTEGER>" << std::endl;
  std::cout << "\t --inputTreeFile <ROOT FILENAME>" << std::endl;
  std::cout << "\t --minimumSensitiveArea <DOUBLE> [m^2] :\t default 0.0"
            << std::endl;
  std::cout << "\t --maximumTreeTrials <INTEGER> :\t default 1000" << std::endl;
}

/*! \brief Efficient tree search main test.
 *
 * Provides an example of how to perform a random search with simple efficiency
 * evaluation. Also can see how to persist the trees en masse.
 *
 * @param[in] argc Number of command line arguments.
 * @param[in] argv Accepts a number of command line arguments. The GEANT4 seed
 *and the starting offset
 *                 for randomizing tree and leaf parmaeters. These default to 1.
 *The number of trees,
 *                 which defaults to 100. The number of time points which
 *defaults to 50. The number of
 *                 photons to generate in each time segment, which defaults to
 *500. The type of
 *                 tree and leaf may also be specified.
 *
 */
int main(int argc, char** argv) {
  std::string treeType, leafType;
  unsigned int treeNumber;
  unsigned int simulationTimeSegments;
  unsigned int photonNumberPerTimeSegment;
  int geant4Seed;
  int parameterSeedOffset;
  std::string inputTreeFileName;
  bool singleTreeRunning = false;
  double minimumSensitiveArea;
  unsigned int maximumTreeTrials;

  GetOpt::GetOpt_pp ops(argc, argv);

  // Check for help request
  if (ops >> GetOpt::OptionPresent('h', "help")) {
    showHelp();
    return 0;
  }

  ops >> GetOpt::Option('t', "tree", treeType, "stump");
  ops >> GetOpt::Option('l', "leaf", leafType, "planar");
  ops >> GetOpt::Option("treeNumber", treeNumber, 100u);
  ops >> GetOpt::Option("timeSegments", simulationTimeSegments, 50u);
  ops >> GetOpt::Option("photonNumber", photonNumberPerTimeSegment, 500u);
  ops >> GetOpt::Option("geant4Seed", geant4Seed, 1);
  ops >> GetOpt::Option("parameterSeedOffset", parameterSeedOffset, 1);
  ops >> GetOpt::Option("inputTreeFile", inputTreeFileName, "");
  ops >> GetOpt::Option("minimumSensitiveArea", minimumSensitiveArea, 0.0);
  ops >> GetOpt::Option("maximumTreeTrials", maximumTreeTrials, 1000u);

  // Report input parameters
  if (inputTreeFileName != "") {
    std::cout << "Just using selected tree from " << inputTreeFileName
              << std::endl;
    singleTreeRunning = true;
  } else {
    std::cout << "Tree type = " << treeType << std::endl;
    std::cout << "Leaf type = " << leafType << std::endl;
    std::cout << "Using the parameter random number seed offset = "
              << parameterSeedOffset << std::endl;
    std::cout << "Generating " << treeNumber << " trees." << std::endl;
    singleTreeRunning = false;
  }
  std::cout << "Using the Geant4 random number seed = " << geant4Seed
            << std::endl;
  std::cout << "Simulating in " << simulationTimeSegments << " time segments."
            << std::endl;
  std::cout << "Considering " << photonNumberPerTimeSegment
            << " photons per time segments." << std::endl;

  // Also do not run if other arguments are present
  if (ops.options_remain()) {
    std::cerr << "Oops! Unexpected options." << std::endl;
    showHelp();
    return -1;
  }

  pvtree::loadEnvironment();

  // Prepare initial conditions for test trunk and leaves
  std::shared_ptr<TreeConstructionInterface> tree;
  std::shared_ptr<LeafConstructionInterface> leaf;

  if (!singleTreeRunning) {
    tree = TreeFactory::instance()->getTree(treeType);
    leaf = LeafFactory::instance()->getLeaf(leafType);
  } else {
    TFile inputTreeFile(inputTreeFileName.c_str(), "READ");
    tree = std::shared_ptr<TreeConstructionInterface>(
        (TreeConstructionInterface*)inputTreeFile.FindObjectAny(
            "selectedTree"));
    leaf = std::shared_ptr<LeafConstructionInterface>(
        (LeafConstructionInterface*)inputTreeFile.FindObjectAny(
            "selectedLeaf"));
    inputTreeFile.Close();
  }

  // Get the device location details
  LocationDetails deviceLocation("location.cfg");

  // Set the altitude of the spectrum factory using location details
  SpectrumFactory::instance()->setAltitude(deviceLocation.getAltitude());

  // Prepare the climate factory with the default configuration
  ClimateFactory::instance()->setConfigurationFile("default.cfg");
  ClimateFactory::instance()->setDeviceLocation(deviceLocation);

  // Define the sun setting, just an arbitrary date for now
  // Perform the simulation between the sunrise and sunset.
  Sun sun(deviceLocation);
  //  sun.setDate(190, 2014); // summer
  sun.setDate(19, 2014);  // winter
  int simulationStartingTime = sun.getSunriseTime() * 60;  // s
  int simulationEndingTime = sun.getSunsetTime() * 60;  // s,
  int simulationStepTime =
      (double)(simulationEndingTime - simulationStartingTime) /
      simulationTimeSegments;

  std::cout << "Simulation time considered between " << simulationStartingTime
            << "(s) and " << simulationEndingTime << "(s)." << std::endl;

  // Set the default materials to be used
  MaterialFactory::instance()->addConfigurationFile("defaults-tree.cfg");

  // Setup Geant4
  G4Random::setTheEngine(new CLHEP::RanecuEngine);
  G4Random::setTheSeed(geant4Seed);

  G4RunManager* runManager = new G4RunManager;

  // Set mandatory initialization classes
  DetectorConstruction* detector = new DetectorConstruction(tree, leaf);
  runManager->SetUserInitialization(detector);

  // Construct a recorder to obtain results
  ConvergenceRecorder recorder;

  OpticalPhysicsList* physicsList = new OpticalPhysicsList;
  runManager->SetUserInitialization(physicsList);

  // Setup primary generator to initialize for the simulation
  runManager->SetUserInitialization(new ActionInitialization(
      &recorder,
      [&photonNumberPerTimeSegment, &sun ]() -> G4VUserPrimaryGeneratorAction *
      {
        return new PrimaryGeneratorAction(photonNumberPerTimeSegment, &sun);
      }));

  // Initialize G4 kernel
  runManager->Initialize();

  // Prepare a root file to store the results
  TFile resultsFile("treeScan.results.root", "RECREATE");

  // Make a TList to store some trees
  TList exportList;
  resultsFile.Add(&exportList);

  //   TList treeExportList;
  //   TList leafExportList;

  // Setup a signal handler to catch batch job + user terminations
  // so that we can still try to output some of the results.
  // SIGINT == 2 (Ctrl-C on command line)
  // TERM_RUNLIMIT on LSF uses User Defined Signal 2 == 12
  SignalReceiver::instance()->setSignals(
      {2, 12}, [&resultsFile, &exportList](int signum) {
        printf("Caught a signal %d\n", signum);

        // Write results out to the root file
        resultsFile.cd();
        exportList.Write("testedStructures", TObject::kSingleKey);
        //       treeExportList.Write("testedTrees",  TObject::kSingleKey);
        //       leafExportList.Write("testedLeaves", TObject::kSingleKey);

        // Close the root file
        resultsFile.Close();

        printf("Attempted to write root file with %d trees.\n",
               exportList.GetSize());

        // Terminate program
        exit(signum);
      });

  // Repeat for a number of trees
  unsigned int currentTreeNumber = 0u;
  unsigned int treeTrialNumber = 0u;
  double totalNormal;
  double totalDiffuse;
  double totalInitial = 0.0;
  while (currentTreeNumber < treeNumber &&
         treeTrialNumber < maximumTreeTrials) {
    treeTrialNumber++;
    //  for (unsigned int x=0; x<treeNumber; x++){

    if (!singleTreeRunning) {
      // Allow the geometry to be rebuilt with new settings
      tree->randomizeParameters(treeTrialNumber + parameterSeedOffset);
      leaf->randomizeParameters(treeTrialNumber + parameterSeedOffset);

      detector->resetGeometry(tree, leaf);
      runManager->ReinitializeGeometry(true, false);         // clean up
      runManager->DefineWorldVolume(detector->Construct());  // reconstruction

      // Lets not bother with small surface areas!
      if (detector->getSensitiveSurfaceArea() < minimumSensitiveArea) {
        continue;
      }
    }

    if (currentTreeNumber % 50 == 0) {
      std::cout << "Considering tree " << currentTreeNumber << std::endl;
      tree->print();
      leaf->print();
    }

    // Simulate at all time points with the same number of events...
    totalInitial = 0.0;
    for (unsigned int timeIndex = 0; timeIndex < simulationTimeSegments;
         timeIndex++) {
      // Set the time to the mid-point of the time segment
      sun.setTime((int)(simulationStartingTime +
                        timeIndex * simulationStepTime +
                        simulationStepTime / 2.0));

      // Run simulation with a single event per time point
      G4int eventNumber = 1;
      runManager->BeamOn(eventNumber);

      auto normalIrradianceHistogram =
          sun.getSpectrum()->getHistogram("Direct_normal_irradiance");
      auto diffuseIrradianceHistogram =
          sun.getSpectrum()->getHistogram("Difuse_horizn_irradiance");
      totalNormal = normalIrradianceHistogram->Integral("width");    // [W/m^2]
      totalDiffuse = diffuseIrradianceHistogram->Integral("width");  // [W/m^2]
      totalInitial +=
          (totalNormal + totalDiffuse) / 1000.0 *
          (simulationStepTime / 3600.0);  // sum over all time slices
    }

    // Get the total surface area which is "sensitive" from current tested
    // detector.
    double sensitiveArea = detector->getSensitiveSurfaceArea();

    // Get the number of leaves
    int numberOfLeaves = detector->getNumberOfLeaves();
    int numberOfRejectedLeaves = detector->getNumberOfRejectedLeaves();

    // Get size of the rough bounding box structure along the axis
    double structureXSize = detector->getXSize();
    double structureYSize = detector->getYSize();
    double structureZSize = detector->getZSize();

    // Sum up the energy deposited (in KiloWatt-Hours)
    double totalEnergyDeposited = 0.0;
    std::vector<std::vector<double> > hitEnergies =
        recorder.getSummedHitEnergies();

    for (unsigned int timeIndex = 0; timeIndex < simulationTimeSegments;
         timeIndex++) {
      for (double eventHitEnergy : hitEnergies[timeIndex]) {
        totalEnergyDeposited +=
            (eventHitEnergy / 1000.0) * (simulationStepTime / 3600.0);
      }
    }

    // Don't need to keep old records after analysis performed.
    recorder.reset();

    std::cout << "Scored Energy [kWh] " << totalEnergyDeposited << std::endl;
    //    std::cout << "from Initial [kWh/m^2] " << totalInitial << std::endl;
    //     std::cout << "on Area [m^2] " << sensitiveArea << std::endl;

    // Clone the settings/results before moving onto next tree so that they can
    // be saved at the end.
    std::string treeName =
        "tree" + std::to_string(currentTreeNumber + parameterSeedOffset);
    TreeConstructionInterface* clonedTree =
        (TreeConstructionInterface*)tree->Clone(treeName.c_str());

    // Store additional information in the cloned tree for later analysis
    clonedTree->setParameter("sensitiveArea", sensitiveArea);
    clonedTree->setParameter("leafNumber", numberOfLeaves);
    clonedTree->setParameter("rejectedLeafNumber", numberOfRejectedLeaves);
    clonedTree->setParameter("structureXSize", structureXSize);
    clonedTree->setParameter("structureYSize", structureYSize);
    clonedTree->setParameter("structureZSize", structureZSize);
    clonedTree->setParameter("totalInitial", totalInitial);
    clonedTree->setParameter("totalEnergy", totalEnergyDeposited);

    std::string leafName =
        "leaf" + std::to_string(currentTreeNumber + parameterSeedOffset);
    LeafConstructionInterface* clonedLeaf =
        (LeafConstructionInterface*)leaf->Clone(leafName.c_str());

    // Add to the list that will be exported
    YearlyResult* result = new YearlyResult();

    result->setTree(clonedTree);
    result->setLeaf(clonedLeaf);
    exportList.Add(result);

    // Move onto next tree
    currentTreeNumber++;
    // Add to the list that will be exported
    //     treeExportList.Add(clonedTree);
    //     leafExportList.Add(clonedLeaf);
  }

  // Job termination
  delete runManager;

  // Write results out to the root file
  resultsFile.cd();
  exportList.Write("testedStructures", TObject::kSingleKey);
  //   treeExportList.Write("testedTrees",  TObject::kSingleKey);
  //   leafExportList.Write("testedLeaves", TObject::kSingleKey);

  // Close the root file
  resultsFile.Close();

  return 0;
}
