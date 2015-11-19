/*!
 * @file 
 * \brief Application to record the performance of many trees
 *        over a period of many days.
 *
 */

#include "treeSystem/treeFactory.hpp"
#include "leafSystem/leafFactory.hpp"
#include "full/detectorConstruction.hpp"
#include "full/actionInitialization.hpp"
#include "full/primaryGeneratorAction.hpp"
#include "full/opticalPhysicsList.hpp"
#include "recorders/convergenceRecorder.hpp"
#include "solarSimulation/sun.hpp"
#include "analysis/yearlyResult.hpp"
#include "material/materialFactory.hpp"
#include "utils/getopt_pp.h"
#include "climate/climateFactory.hpp"
#include "location/locationDetails.hpp"
#include "solarSimulation/spectrumFactory.hpp"
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
#include "TGraphAsymmErrors.h"
#include "TAxis.h"
#include "TCanvas.h"
#include "TH1F.h"

// turn the warnings back on
#pragma GCC diagnostic pop

void showHelp() {
  std::cout << "yearlyTreeScan help" << std::endl;
  std::cout << "\t -t, --tree <TREE TYPE NAME> :\t default 'stump'" << std::endl;
  std::cout << "\t -l, --leaf <LEAF TYPE NAME> :\t default 'planar'" << std::endl;
  std::cout << "\t --treeNumber <INTEGER> :\t default 10" << std::endl;
  std::cout << "\t --maximumTreeTrials <INTEGER> :\t default 1000" << std::endl;
  std::cout << "\t --timeSegments <INTEGER> :\t default 50" << std::endl;
  std::cout << "\t --photonNumber <INTEGER> :\t default 500" << std::endl;
  std::cout << "\t --geant4Seed <INTEGER> :\t default 1" << std::endl;
  std::cout << "\t --parameterSeed <INTEGER> :\t default 1" << std::endl;
  std::cout << "\t --inputTreeFile <ROOT FILENAME> :\t default ''" << std::endl;
  std::cout << "\t --startDate <INTEGER> :\t default 1/1/2014" << std::endl;
  std::cout << "\t --endDate <INTEGER> :\t default 1/1/2015" << std::endl;
  std::cout << "\t --yearSegments <INTEGER> :\t default 10" << std::endl;
  std::cout << "\t --minimumSensitveArea <DOUBLE> [m^2] :\t default 0.0" << std::endl;
  std::cout << "\t --outputFileName <ROOT FILENAME> : \t default 'yearlyTreeScan.results.root'" << std::endl;
}

bool isSameDay(time_t time1, time_t time2) {

  // Convert to calendar time
  struct tm* calendarTime = gmtime(&time1);

  int monthDay1 = calendarTime->tm_mday;
  int month1 = calendarTime->tm_mon;
  int year1 = calendarTime->tm_year;

  calendarTime = gmtime(&time2);

  int monthDay2 = calendarTime->tm_mday;
  int month2 = calendarTime->tm_mon;
  int year2 = calendarTime->tm_year;

  if (monthDay1 == monthDay2 && month1 == month2 && year1 == year2){
    return true;
  }
  return false;
}

/*! \brief Convert date in format DD/MM/YYYY into the time
 *         since epoch.
 *
 *
 * @param[in] inputDate String representing date in format DD/MM/YYYY
 * \returns The time since epoch.
 */
time_t interpretDate(std::string inputDate) {

  // Use regular expressions to extract important quantities
  std::regex regularExpression("(\\d+)/(\\d+)/(\\d+)");

  std::smatch dateMatches;
  std::regex_match(inputDate, dateMatches, regularExpression);

  // Is the match the correct size?
  if ( dateMatches.size() != 4 ){
    throw std::string("Cannot interpret date: " + inputDate);
  }

  struct tm calendarTime;

  calendarTime.tm_sec = 0;
  calendarTime.tm_min = 0; 
  calendarTime.tm_hour = 12; 
  calendarTime.tm_mday = std::stoi(dateMatches[1]); 
  calendarTime.tm_mon =  std::stoi(dateMatches[2])-1; 
  calendarTime.tm_year = std::stoi(dateMatches[3])-1900;
  calendarTime.tm_isdst = 1;

  return mktime(&calendarTime);  
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
  unsigned int simulationTimeSegments;
  unsigned int photonNumberPerTimeSegment;
  int geant4Seed;
  int parameterSeed;
  std::string inputTreeFileName;
  bool singleTreeRunning = false;
  std::string startDate;
  std::string endDate;
  unsigned int yearSegments;
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
  ops >> GetOpt::Option("timeSegments", simulationTimeSegments, 50u);
  ops >> GetOpt::Option("photonNumber", photonNumberPerTimeSegment, 500u);
  ops >> GetOpt::Option("geant4Seed", geant4Seed, 1);
  ops >> GetOpt::Option("parameterSeed", parameterSeed, 1);
  ops >> GetOpt::Option("inputTreeFile", inputTreeFileName, "");
  ops >> GetOpt::Option("startDate", startDate, "1/1/2014");
  ops >> GetOpt::Option("endDate",   endDate,   "1/1/2015");
  ops >> GetOpt::Option("yearSegments", yearSegments, 10u);
  ops >> GetOpt::Option("minimumSensitiveArea", minimumSensitiveArea, 0.0);
  ops >> GetOpt::Option("outputFileName", outputFileName, "yearlyTreeScan.results.root");

  if (yearSegments == 0) {
    std::cerr << "Need at least one year time segment." << std::endl;
    return -1;
  }

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
  std::cout << "Using the Geant4 random number seed = "           << geant4Seed << std::endl;
  std::cout << "Simulating in " << simulationTimeSegments << " time segments." << std::endl;
  std::cout << "Considering "   << photonNumberPerTimeSegment << " photons per time segments." << std::endl;
  std::cout << "Starting from day " << startDate << " and finishing on " << endDate << " splitting into " 
	    << yearSegments << " segments." << std::endl;
  std::cout << "Recording results in " << outputFileName << std::endl;

  // Also do not run if other arguments are present
  if (ops.options_remain()) {
    std::cerr << "Oops! Unexpected options." << std::endl;
    showHelp();
    return -1;
  }

  // Attempt to interpret the start and end dates.
  time_t interpretedStartDate = interpretDate(startDate);
  time_t interpretedEndDate =   interpretDate(endDate);

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

  // Get the device location details
  LocationDetails deviceLocation("location.cfg");

  // Set the altitude of the spectrum factory using location details
  SpectrumFactory::instance()->setAltitude(deviceLocation.getAltitude());

  // Prepare the climate factory with the default configuration
  ClimateFactory::instance()->setConfigurationFile("default.cfg");
  ClimateFactory::instance()->setDeviceLocation(deviceLocation);

  // Obtain the simulation sun
  Sun sun(deviceLocation);
  sun.setClimateOption(Sun::CLOUDCOVER, false); // Ignore clouds!

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

  // Set mandatory initialization classes  
  DetectorConstruction* detector = new DetectorConstruction(tree, leaf);
  runManager->SetUserInitialization(detector);

  //Construct a recorder to obtain results
  ConvergenceRecorder recorder;

  OpticalPhysicsList* physicsList = new OpticalPhysicsList;
  runManager->SetUserInitialization(physicsList);

  // Setup primary generator to initialize for the simulation
  runManager->SetUserInitialization(new ActionInitialization(&recorder, 
   [&photonNumberPerTimeSegment, &sun] () -> G4VUserPrimaryGeneratorAction* { 
							       return new PrimaryGeneratorAction(photonNumberPerTimeSegment, &sun); }) );

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


  // Repeat for a number of trees
  // Use a maximum trial limit to prevent infinite loop
  unsigned int currentTreeNumber = 0u;
  unsigned int treeTrialNumber = 0u;
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

    std::vector<time_t > dayTimes;
    std::vector<double > dayEnergySums;

    // Create a list of days (avoiding duplication).
    double yearSegmentSize = (interpretedEndDate-interpretedStartDate)/yearSegments;

    for (unsigned int segmentIndex=0; segmentIndex<yearSegments+1; segmentIndex++ ) {

      time_t candidateDay = interpretedStartDate + yearSegmentSize*segmentIndex;

      // Check that it is on a different day
      if ( dayTimes.size() > 0 && isSameDay(candidateDay, dayTimes.back()) ) {
	continue;
      }

      dayTimes.push_back(candidateDay);
    }

    double totalEvaluatedEnergy = 0.0;

    // Repeat simulation for each day
    for (unsigned int dayIndex=0; dayIndex<dayTimes.size(); dayIndex++) {

      // Perform the simulation between the sunrise and sunset on the selected day.
      sun.setDate(dayTimes[dayIndex]);
      int simulationStartingTime = sun.getSunriseTime()*60; //s
      int simulationEndingTime   = sun.getSunsetTime()*60; //s, 
      int simulationStepTime     = (double)(simulationEndingTime-simulationStartingTime)/simulationTimeSegments;

      // Integrate over the representative day
      // Simulate at all time points with the same number of events...
      for (unsigned int timeIndex=0; timeIndex<simulationTimeSegments; timeIndex++) {
      
	// Set the time to the mid-point of the day-time segment
	sun.setTime( (int)(simulationStartingTime + timeIndex*simulationStepTime + simulationStepTime/2.0) );
      
	// Run simulation with a single event per time point
	G4int eventNumber = 1;
	runManager->BeamOn(eventNumber);
      }

      // Sum up the energy deposited (in KiloWatt hour)
      std::vector<std::vector<double > > hitEnergies = recorder.getSummedHitEnergies();

      // Grab the total energy sum firstly for normalization!
      double totalEnergy = 0.0;
      
      for (unsigned int timeIndex=0; timeIndex<simulationTimeSegments; timeIndex++) {
	
	// Sum up the energy deposited in a 'run' (should just be a single event per run).
	double totalRunEnergy = 0.0; //kWh
	for (double eventHitEnergy : hitEnergies[timeIndex]) {
	  totalRunEnergy += (eventHitEnergy/1000.0) * (simulationStepTime/3600.0);
	}

	totalEnergy += totalRunEnergy;
	totalEvaluatedEnergy += totalRunEnergy;
      }

      dayEnergySums.push_back(totalEnergy);

      // Don't need to keep old records after analysis performed.
      recorder.reset();
    }

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
    YearlyResult* result = new YearlyResult();

    result->setTree(clonedTree);
    result->setLeaf(clonedLeaf);
    result->setDayTimes(dayTimes);
    result->setEnergyDeposited(dayEnergySums);

    // Sum up the energy deposited (in KiloWatt-Hours), where interpolation
    // is carried out to fill in the gaps.
    double totalEnergyDeposited = result->getEnergyIntegral();

    // Also attach this to the tree
    clonedTree->setParameter("totalEnergy"   ,       totalEnergyDeposited);
    clonedTree->setParameter("totalEvaluatedEnergy", totalEvaluatedEnergy);

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
  std::cout <<  currentTreeNumber << " trees produced in " << treeTrialNumber << " trials." << std::endl; 

  if (!(treeTrialNumber < maximumTreeTrials)) {
    std::cerr << "Not a sufficient number of trials available to satisfy tree demand" << std::endl;
    return 1;
  }

  return 0;
}







