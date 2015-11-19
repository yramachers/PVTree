/*!
 * @file 
 * \brief Application to record the performance of many trees
 *        when subjected to a lightfield.
 *
 */

#include "treeSystem/treeFactory.hpp"
#include "leafSystem/leafFactory.hpp"
#include "solarSimulation/plenoptic3D.hpp"
#include "full/detectorConstruction.hpp"
#include "full/actionInitialization.hpp"
#include "full/lightfieldGeneratorAction.hpp"
#include "full/opticalPhysicsList.hpp"
#include "recorders/convergenceRecorder.hpp"
#include "analysis/yearlyResult.hpp"
#include "material/materialFactory.hpp"
#include "utils/getopt_pp.h"
#include "utils/signalReceiver.hpp"

#include <iostream>
#include <vector>
#include <memory>
#include <chrono>
#include <sstream>
#include <random>
#include <csignal>
#include <regex>

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
#include "TRandom.h"

// turn the warnings back on
#pragma GCC diagnostic pop

void showHelp() {
  std::cout << "lightfieldTreeScan help" << std::endl;
  std::cout << "\t -t, --tree <TREE TYPE NAME> :\t default 'stump'" << std::endl;
  std::cout << "\t -l, --leaf <LEAF TYPE NAME> :\t default 'planar'" << std::endl;
  std::cout << "\t --treeNumber <INTEGER> :\t default 10" << std::endl;
  std::cout << "\t --maximumTreeTrials <INTEGER> :\t default 1000" << std::endl;
  std::cout << "\t --photonNumberPerEvent <INTEGER> :\t default 500" << std::endl;
  std::cout << "\t --eventNumber <INTEGER> :\t default 1" << std::endl;
  std::cout << "\t --geant4Seed <INTEGER> :\t default 1" << std::endl;
  std::cout << "\t --parameterSeed <INTEGER> :\t default 1" << std::endl;
  std::cout << "\t --inputTreeFile <ROOT FILENAME> :\t default ''" << std::endl;
  std::cout << "\t --lightfieldFileName <ROOT FILENAME> :\t default 'lightfield.root'" << std::endl;
  std::cout << "\t --minimumSensitveArea <DOUBLE> [m^2] :\t default 0.0" << std::endl;
  std::cout << "\t --outputFileName <ROOT FILENAME> : \t default 'lightfieldTreeScan.results.root'" << std::endl;
}

/*! \brief Efficient tree search main test. 
 *
 * Provides an example of how to perform a random search with simple efficiency
 * evaluation. Also can see how to persist the trees en masse.
 *
 * @param[in] argc Number of command line arguments.
 * @param[in] argv Accepts a number of command line arguments. See help for more 
 *                 details
 * 
 */
int main(int argc, char** argv) {

  std::string treeType, leafType;
  unsigned int treeNumber;
  unsigned int maximumTreeTrials;
  unsigned int photonNumberPerEvent;
  unsigned int eventNumber;
  int geant4Seed;
  int parameterSeed;
  std::string inputTreeFileName;
  std::string lightfieldFileName;
  bool singleTreeRunning = false;
  std::string outputFileName;
  double minimumSensitiveArea;

  GetOpt::GetOpt_pp ops(argc, argv);

  // Check for help request
  if (ops >> GetOpt::OptionPresent('h', "help")){
    showHelp();
    return 0;
  }

  ops >> GetOpt::Option('t', "tree", treeType, "stump");
  ops >> GetOpt::Option('l', "leaf", leafType, "planar");
  ops >> GetOpt::Option("treeNumber", treeNumber, 10u);
  ops >> GetOpt::Option("maximumTreeTrials", maximumTreeTrials, 1000u);
  ops >> GetOpt::Option("photonNumberPerEvent", photonNumberPerEvent, 500u);
  ops >> GetOpt::Option("eventNumber", eventNumber, 1u);
  ops >> GetOpt::Option("geant4Seed", geant4Seed, 1);
  ops >> GetOpt::Option("parameterSeed", parameterSeed, 1);
  ops >> GetOpt::Option("inputTreeFile", inputTreeFileName, "");
  ops >> GetOpt::Option("lightfieldFileName", lightfieldFileName, "lightfield.root");
  ops >> GetOpt::Option("minimumSensitiveArea", minimumSensitiveArea, 0.0);
  ops >> GetOpt::Option("outputFileName", outputFileName, "lightfieldTreeScan.results.root");

  // Report input parameters
  if (inputTreeFileName != "") {
    std::cout << "Just using selected tree from " << inputTreeFileName << std::endl;
    singleTreeRunning = true;
  } else {
    std::cout << "Tree type = " << treeType << std::endl;
    std::cout << "Leaf type = " << leafType << std::endl;
    std::cout << "Using the parameter random number seed = " << parameterSeed << std::endl;
    std::cout << "Generating "    << treeNumber << " trees with up to " << maximumTreeTrials 
	      << " trials." << std::endl;
    singleTreeRunning = false;
  }
  std::cout << "Using the Geant4 random number seed = " << geant4Seed << std::endl;
  std::cout << "Considering "   << photonNumberPerEvent << " photons per event." << std::endl;
  std::cout << "Taking average of " << eventNumber << " events per tree." << std::endl;
  std::cout << "Recording results in " << outputFileName << std::endl;
  std::cout << "Using lightfield defined in " << lightfieldFileName << " to generate photons." << std::endl;

  // Also do not run if other arguments are present
  if (ops.options_remain()) {
    std::cerr << "Oops! Unexpected options." << std::endl;
    showHelp();
    return -1;
  }

  // Load the lightfield to be used
  TFile currentLightfieldFile(lightfieldFileName.c_str(), "READ");
  Plenoptic3D* currentLightfield = static_cast<Plenoptic3D*>( currentLightfieldFile.Get("lightfield") );

  // Set the random number seed for the lightfield
  currentLightfield->setRandomNumberSeedSequence(std::vector<int>{ geant4Seed, parameterSeed, 1501 });

  // Prepare initial conditions for test trunk and leaves
  std::shared_ptr<TreeConstructionInterface> tree;
  std::shared_ptr<LeafConstructionInterface> leaf;

  if (!singleTreeRunning) {
    tree = TreeFactory::instance()->getTree(treeType);
    leaf = LeafFactory::instance()->getLeaf(leafType);
  } else {
    TFile inputTreeFile(inputTreeFileName.c_str(), "READ");
    tree = std::shared_ptr<TreeConstructionInterface>( (TreeConstructionInterface*)inputTreeFile.FindObjectAny("selectedTree") );
    leaf = std::shared_ptr<LeafConstructionInterface>( (LeafConstructionInterface*)inputTreeFile.FindObjectAny("selectedLeaf") );
    inputTreeFile.Close();
  }

  // Set the default materials to be used
  MaterialFactory::instance()->addConfigurationFile("defaults-tree.cfg");

  // Prepare a random number generator for L-System randomization
  std::default_random_engine parameterRandomGenerator((int)parameterSeed);

  // generates number in the range 0..numeric_limits<result_type>::max()
  std::uniform_int_distribution<int> parameterSeedDistribution;

  // Setup Geant4
  G4Random::setTheEngine(new CLHEP::RanecuEngine);
  G4Random::setTheSeed(geant4Seed);

  G4RunManager* runManager = new G4RunManager;

  // Set the root seed
  gRandom->SetSeed(geant4Seed);

  // Set mandatory initialization classes  
  DetectorConstruction* detector = new DetectorConstruction(tree, leaf);
  runManager->SetUserInitialization(detector);

  // Construct a recorder to obtain results
  ConvergenceRecorder recorder;

  OpticalPhysicsList* physicsList = new OpticalPhysicsList;
  runManager->SetUserInitialization(physicsList);

  // Setup primary generator to initialize for the simulation
  runManager->SetUserInitialization(new ActionInitialization(&recorder, 
   [&photonNumberPerEvent, &currentLightfield] () -> G4VUserPrimaryGeneratorAction* { return new LightfieldGeneratorAction(photonNumberPerEvent, currentLightfield); }) );

  // Initialize G4 kernel
  runManager->Initialize();

  // Prepare a root file to store the results
  TFile resultsFile(outputFileName.c_str(), "RECREATE");

  // Make a TList to store some analysis results
  TList exportList;

  // Setup a signal handler to catch batch job + user terminations 
  // so that we can still try to output some of the results.
  // SIGINT == 2 (Ctrl-C on command line)
  // TERM_RUNLIMIT on LSF uses User Defined Signal 2 == 12
  SignalReceiver::instance()->setSignals({2, 12}, [&resultsFile,&exportList](int signum) {   
      printf("Caught a signal %d\n",signum);

      // Write results out to the root file
      exportList.Write("testedStructures",  TObject::kSingleKey);

      // Close the root file
      resultsFile.Close();

      printf("Attempted to write root file with %d trees.\n", exportList.GetSize());

      // Terminate program
      exit(signum); }
    );


  // Get the pointer to the User Interface manager
  G4UImanager* UImanager = G4UImanager::GetUIpointer();
  UImanager->ApplyCommand("/run/verbose 0");
  UImanager->ApplyCommand("/control/verbose 0");
  UImanager->ApplyCommand("/tracking/verbose 0");

  // Repeat for a number of trees
  // Use a maximum trial limit to prevent infinite loop
  unsigned int currentTreeNumber = 0u;
  unsigned int treeTrialNumber = 0u;
  unsigned int failedRuns = 0u;
  while( currentTreeNumber < treeNumber && treeTrialNumber < maximumTreeTrials) {
    treeTrialNumber++;

    if (!singleTreeRunning){
      unsigned int treeParameterSeed =  parameterSeedDistribution(parameterRandomGenerator);
      unsigned int leafParameterSeed =  parameterSeedDistribution(parameterRandomGenerator);
      
      // Allow the geometry to be rebuilt with new settings
      tree->randomizeParameters(treeParameterSeed);
      leaf->randomizeParameters(leafParameterSeed);

      detector->resetGeometry(tree, leaf);

      // Re-initialize the detector geometry
      G4bool destroyFirst;
      runManager->ReinitializeGeometry(destroyFirst = true);

      // Apply pre-selection to the tree after manual construction.
      //      detector->Construct();
      runManager->Initialize();

      // Lets not bother with small surface areas!
      if (detector->getSensitiveSurfaceArea() < minimumSensitiveArea) {
	continue;
      }
    }

    // Simulate a run containing eventNumber events.
    runManager->BeamOn(eventNumber);

    if (recorder.wasAborted()) {
      std::cerr << "Event in run was aborted, who knows why!" << std::endl;
      failedRuns++;
      recorder.reset();
      continue;
    }

    // Calculate the average energy deposited in the single run. (in KiloWatt hour)
    std::vector<std::vector<double > > hitEnergies = recorder.getSummedHitEnergies();
    std::vector<std::vector<long > >   photonCounts = recorder.getPhotonCounts();

    if (hitEnergies.size() != 1) {
      // Not expecting there to be more than one run ..
      std::cerr << "Wrong number of runs in hit energy vector. Giving up!" << std::endl;
      return 1;
    }

    // Check that the correct number of events has been produced
    if (hitEnergies[0].size() != eventNumber) {
      // Something has gone wrong
      std::cerr << "Did not simulate the expected number of events." << std::endl;
      std::cerr << "Actually produced " << hitEnergies[0].size() << " events." << std::endl;
      failedRuns++;
      recorder.reset();
      continue;
    }

    // Check each event had the correct number of photons produced
    if (photonCounts.size() != 1 ) {
      std::cerr << "Wrong number of runs in hit energy vector. Giving up!" << std::endl;
      return 1;
    }
    if (photonCounts[0].size() != eventNumber) {
      // Something has gone wrong
      std::cerr << "Did not simulate the expected number of events." << std::endl;
      std::cerr << "Actually produced " << photonCounts[0].size() << " events." << std::endl;
      failedRuns++;
      recorder.reset();
      continue;
    }
    bool badPhotonCount = false;
    for (long photonCount : photonCounts[0]){
      if (photonCount != photonNumberPerEvent) {
	badPhotonCount = true;
	std::cerr << "Only produced " << badPhotonCount << " photons in this event. Something bad happened." << std::endl;
	break;
      }
    }
    if (badPhotonCount) {
      failedRuns++;
      recorder.reset();
      continue;
    }

    double trialEnergySum = 0.0;
    for (double eventHitEnergy : hitEnergies[0]) {
      trialEnergySum += eventHitEnergy;
    }
    double averageEnergySum = trialEnergySum / hitEnergies[0].size();

    // Also calculate the sample standard deviaition of the events
    double energyStandardDeviationSum = 0.0;
    for (double eventHitEnergy : hitEnergies[0]) {
      energyStandardDeviationSum = std::pow(averageEnergySum - eventHitEnergy, 2.0);
    }
    double energyStandardDeviation = std::sqrt( energyStandardDeviationSum / (hitEnergies[0].size()-1) );

    // Convert to kWh
    averageEnergySum = averageEnergySum / (3600.0*1000.0);
    energyStandardDeviation = energyStandardDeviation / (3600.0*1000.0);

    // Remove the old records
    recorder.reset();

    // Get the total surface area which is "sensitive" from current tested detector.
    double sensitiveArea = detector->getSensitiveSurfaceArea();
    
    // Get the number of leaves
    int    numberOfLeaves = detector->getNumberOfLeaves();
    int    numberOfRejectedLeaves = detector->getNumberOfRejectedLeaves();

    // Get size of the rough bounding box structure along the axis
    double structureXSize = detector->getXSize();
    double structureYSize = detector->getYSize();
    double structureZSize = detector->getZSize();

    // Clone the settings/results before moving onto next tree so that they can be saved at the end.
    std::string treeName = "tree" + std::to_string(currentTreeNumber) + "_Job" + std::to_string(parameterSeed);
    TreeConstructionInterface* clonedTree = (TreeConstructionInterface*)tree->Clone(treeName.c_str());
    
    // Store additional information in the cloned tree for later analysis
    clonedTree->setParameter("sensitiveArea",      sensitiveArea);
    clonedTree->setParameter("leafNumber",         numberOfLeaves);
    clonedTree->setParameter("rejectedLeafNumber", numberOfRejectedLeaves);
    clonedTree->setParameter("structureXSize",     structureXSize);
    clonedTree->setParameter("structureYSize",     structureYSize);
    clonedTree->setParameter("structureZSize",     structureZSize);

    std::string leafName = "leaf" + std::to_string(currentTreeNumber) + "_Job" + std::to_string(parameterSeed);
    LeafConstructionInterface* clonedLeaf = (LeafConstructionInterface*)leaf->Clone(leafName.c_str());

    // Add to the list that will be exported
    // No longer use the majority of the features of YearlyResult...
    YearlyResult* result = new YearlyResult();

    result->setTree(clonedTree);
    result->setLeaf(clonedLeaf);

    // Attach the average total energy from the run
    clonedTree->setParameter("totalEnergy", averageEnergySum);
    clonedTree->setParameter("totalEnergyStdDeviation", energyStandardDeviation);

    exportList.Add(result);

    std::cout << "Considered tree " << currentTreeNumber << " in trial " << treeTrialNumber << std::endl;
    clonedTree->print();
    clonedLeaf->print();

    // Move onto next tree
    currentTreeNumber++;
  }

  // Job termination
  delete runManager;

  // Write results out to the root file
  exportList.Write("testedStructures",  TObject::kSingleKey);

  // Close the root file
  resultsFile.Close();

  // Output some useful information
  std::cout << currentTreeNumber << " trees produced in " << treeTrialNumber << " trials." << std::endl; 
  std::cout << failedRuns << " failed runs." << std::endl;

  if (!(treeTrialNumber < maximumTreeTrials)) {
    std::cerr << "Not a sufficient number of trials available to satisfy tree demand" << std::endl;
    return 1;
  }

  return 0;
}







