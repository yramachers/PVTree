/*!
 * @file
 * \brief Application to investigate the collection efficiency of 
 *        randomly generated forests of identical tree copies over 
 *        the period of one year.
 */

#include "pvtree/treeSystem/treeFactory.hpp"
#include "pvtree/leafSystem/leafFactory.hpp"
#include "pvtree/full/detectorConstruction.hpp"
#include "pvtree/full/actionInitialization.hpp"
#include "pvtree/full/primaryGeneratorAction.hpp"
#include "pvtree/full/opticalPhysicsList.hpp"
#include "pvtree/full/recorders/forestRecorder.hpp"
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
#include "TList.h"
#include "TH1D.h"
#include "TTree.h"

// turn the warnings back on
#pragma GCC diagnostic pop

void showHelp() {
  std::cout << "yearlyForestScan help" << std::endl;
  std::cout << "\t -t, --tree <TREE TYPE NAME> :\t default 'monopodial'" << std::endl;
  std::cout << "\t -l, --leaf <LEAF TYPE NAME> :\t default 'cordate'" << std::endl;
  std::cout << "\t --simulations <INTEGER> :\t default 1" << std::endl;
  std::cout << "\t --treeNumber <INTEGER> :\t default 9" << std::endl;
  std::cout << "\t --timeSegments <INTEGER> :\t default 25" << std::endl;
  std::cout << "\t --photonNumber <INTEGER> :\t default 500" << std::endl;
  std::cout << "\t --geant4Seed <INTEGER> :\t default 1" << std::endl;
  std::cout << "\t --parameterSeed <INTEGER> :\t default 1" << std::endl;
  std::cout << "\t --startDate <INTEGER> :\t default 1/1/2014" << std::endl;
  std::cout << "\t --endDate <INTEGER> :\t default 1/1/2015" << std::endl;
  std::cout << "\t --yearSegments <INTEGER> :\t default 12" << std::endl;
  std::cout << "\t --minimumSensitiveArea <DOUBLE> [m^2] :\t default 1.0"
            << std::endl;
  std::cout << "\t --maximumTreeTrials <INTEGER> :\t default 1000" << std::endl;
  std::cout << "\t --outputFileName <ROOT FILENAME> : \t default "
               "'yearlyForestScan.results.root'" << std::endl;
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

  if (monthDay1 == monthDay2 && month1 == month2 && year1 == year2) {
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
  if (dateMatches.size() != 4) {
    throw std::string("Cannot interpret date: " + inputDate);
  }

  struct tm calendarTime;

  calendarTime.tm_sec = 0;
  calendarTime.tm_min = 0;
  calendarTime.tm_hour = 12;
  calendarTime.tm_mday = std::stoi(dateMatches[1]);
  calendarTime.tm_mon = std::stoi(dateMatches[2]) - 1;
  calendarTime.tm_year = std::stoi(dateMatches[3]) - 1900;
  calendarTime.tm_isdst = 1;

  return mktime(&calendarTime);
}

/*! 
 *
 * @param[in] argc Number of command line arguments.
 * @param[in] argv Accepts a number of command line arguments.
 *                 See help for more details.
 *
 */
int main(int argc, char** argv) {
  std::string treeType, leafType;
  unsigned int simulations;
  unsigned int treeNumber;
  unsigned int simulationTimeSegments;
  unsigned int photonNumberPerTimeSegment;
  int geant4Seed;
  int parameterSeed;
  double minimumSensitiveArea;
  unsigned int maximumTreeTrials;
  std::string startDate;
  std::string endDate;
  unsigned int yearSegments;
  std::string outputFileName;

  GetOpt::GetOpt_pp ops(argc, argv);

  // Check for help request
  if (ops >> GetOpt::OptionPresent('h', "help")) {
    showHelp();
    return 0;
  }

  ops >> GetOpt::Option('t', "tree", treeType, "monopodial");
  ops >> GetOpt::Option('l', "leaf", leafType, "cordate");
  ops >> GetOpt::Option("simulations", simulations, 1u);
  ops >> GetOpt::Option("treeNumber", treeNumber, 9u);
  ops >> GetOpt::Option("timeSegments", simulationTimeSegments, 25u);
  ops >> GetOpt::Option("photonNumber", photonNumberPerTimeSegment, 500u);
  ops >> GetOpt::Option("geant4Seed", geant4Seed, 1);
  ops >> GetOpt::Option("parameterSeed", parameterSeed, 1);
  ops >> GetOpt::Option("startDate", startDate, "1/1/2014");
  ops >> GetOpt::Option("endDate", endDate, "1/1/2015");
  ops >> GetOpt::Option("yearSegments", yearSegments, 12u);
  ops >> GetOpt::Option("minimumSensitiveArea", minimumSensitiveArea, 1.0);
  ops >> GetOpt::Option("maximumTreeTrials", maximumTreeTrials, 1000u);
  ops >> GetOpt::Option("outputFileName", outputFileName,
                        "yearlyForestScan.results.root");

  if (yearSegments == 0) {
    std::cerr << "Need at least one year time segment." << std::endl;
    return -1;
  }


  // Report input parameters
  std::cout << "Tree type = " << treeType << std::endl;
  std::cout << "Leaf type = " << leafType << std::endl;
    std::cout << "Using the parameter random number seed offset = "
              << parameterSeed << std::endl;
  std::cout << "Generating " << treeNumber << " trees per forest." << std::endl;
  std::cout << "in " << simulations << " simulated forests." << std::endl;

  std::cout << "Using the Geant4 random number seed = " << geant4Seed
            << std::endl;
  std::cout << "Simulating in " << simulationTimeSegments << " time segments."
            << std::endl;
  std::cout << "Considering " << photonNumberPerTimeSegment
            << " photons per time segments." << std::endl;
  std::cout << "Starting from day " << startDate << " and finishing on "
            << endDate << " splitting into " << yearSegments << " segments."
            << std::endl;
  std::cout << "Recording results in " << outputFileName << std::endl;

  // Also do not run if other arguments are present
  if (ops.options_remain()) {
    std::cerr << "Oops! Unexpected options." << std::endl;
    showHelp();
    return -1;
  }

  pvtree::loadEnvironment();

  // Attempt to interpret the start and end dates.
  time_t interpretedStartDate = interpretDate(startDate);
  time_t interpretedEndDate = interpretDate(endDate);

  // Prepare initial conditions for test trunk and leaves
  std::shared_ptr<TreeConstructionInterface> tree;
  std::shared_ptr<LeafConstructionInterface> leaf;

  tree = TreeFactory::instance()->getTree(treeType);
  leaf = LeafFactory::instance()->getLeaf(leafType);

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

  // Set the default materials to be used
  MaterialFactory::instance()->addConfigurationFile("defaults-tree.cfg");

  // Prepare a random number generator for L-System randomization
  std::default_random_engine parameterRandomGenerator(parameterSeed);

  // generates number in the range 0..numeric_limits<result_type>::max()
  std::uniform_int_distribution<int> parameterSeedDistribution;

  // Setup Geant4
  G4Random::setTheEngine(new CLHEP::RanecuEngine);
  G4Random::setTheSeed(geant4Seed);

  G4RunManager* runManager = new G4RunManager;

  // Set mandatory initialization classes 
  DetectorConstruction* detector = new DetectorConstruction(tree, leaf, 
      treeNumber);
  runManager->SetUserInitialization(detector);

  // Construct a recorder to obtain results
  ForestRecorder recorder;

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

  // Repeat for a number of trees
  unsigned int currentForestNumber = 0u;
  unsigned int treeID = 0u;
  int xID = 0;
  int yID = 0;
  double energyReceived = 0.0;

  // Prepare a root file to store the results
  TFile resultsFile(outputFileName.c_str(), "RECREATE");
  TTree* forestdata = new TTree("forestData","Store energy per tree");
  forestdata->Branch("simID", &currentForestNumber); // add simID branch to TTree
  forestdata->Branch("treeID", &treeID);
  forestdata->Branch("xID", &xID);
  forestdata->Branch("yID", &yID);
  forestdata->Branch("treeEnergy", &energyReceived);

  // Make a TList to store some trees
  TList exportList;
  resultsFile.Add(&exportList);

  // Setup a signal handler to catch batch job + user terminations
  // so that we can still try to output some of the results.
  // SIGINT == 2 (Ctrl-C on command line)
  // TERM_RUNLIMIT on LSF uses User Defined Signal 2 == 12
  SignalReceiver::instance()->setSignals(
					 {2, 12, 12}, [&resultsFile, &exportList, &forestdata](int signum) {
        printf("Caught a signal %d\n", signum);

        // Write results out to the root file
        resultsFile.cd();
        exportList.Write("testedStructures", TObject::kSingleKey);
	forestdata->Write(); // rescue whatever is there

        // Close the root file
        resultsFile.Close();

        printf("Attempted to write root file with %d trees.\n",
               exportList.GetSize());

        // Terminate program
        exit(signum);
      });

  double totalNormal;
  double totalDiffuse;
  double totalInitial = 0.0;
  unsigned int treeTrialNumber = 0u;

  std::vector<time_t> dayTimes;
  std::vector<double> dayEnergySums;
  std::map<unsigned int, double> yearenergyPerTree;

  while (currentForestNumber < simulations &&
         treeTrialNumber < maximumTreeTrials) {
    treeTrialNumber++;

    unsigned int treeParameterSeed =
      parameterSeedDistribution(parameterRandomGenerator);
    unsigned int leafParameterSeed =
      parameterSeedDistribution(parameterRandomGenerator);

    // Allow the geometry to be rebuilt with new settings
    tree->randomizeParameters(treeParameterSeed);
    leaf->randomizeParameters(leafParameterSeed);

    detector->resetGeometry(tree, leaf, treeNumber);
    //    runManager->GeometryHasBeenModified();
    runManager->ReinitializeGeometry(true, false);         // clean up
    runManager->BeamOn(0); // fake start to build geometry
    //    runManager->DefineWorldVolume(detector->Construct());  // reconstruction

    // Lets not bother with small surface areas!
    if (detector->getSensitiveSurfaceArea() < minimumSensitiveArea) {
      continue;
    }

    // Create a list of days (avoiding duplication).
    double yearSegmentSize =
        (interpretedEndDate - interpretedStartDate) / yearSegments;

    totalInitial = 0.0;
    for (unsigned int segmentIndex = 0; segmentIndex < yearSegments + 1;
         segmentIndex++) {
      time_t candidateDay =
          interpretedStartDate + yearSegmentSize * segmentIndex;

      // Check that it is on a different day
      if (dayTimes.size() > 0 && isSameDay(candidateDay, dayTimes.back())) {
        continue;
      }

      dayTimes.push_back(candidateDay);
    }

    if (currentForestNumber % 50 == 0) {
      std::cout << "Considering forest " << currentForestNumber << std::endl;
      tree->print();
      leaf->print();
    }

    double totalEvaluatedEnergy = 0.0;
    // Repeat simulation for each day
    for (unsigned int dayIndex = 0; dayIndex < dayTimes.size(); dayIndex++) {
      // Perform the simulation between the sunrise and sunset on the selected
      // day.
      sun.setDate(dayTimes[dayIndex]);
      int simulationStartingTime = sun.getSunriseTime() * 60;  // s
      int simulationEndingTime = sun.getSunsetTime() * 60;  // s,
      int simulationStepTime =
          (double)(simulationEndingTime - simulationStartingTime) /
          simulationTimeSegments;

      // Integrate over the representative day
      // Simulate at all time points with the same number of events...
      int dummytime = 0;
      for (unsigned int timeIndex = 0; timeIndex < simulationTimeSegments;
           timeIndex++) {
        // Set the time to the mid-point of the day-time segment
	dummytime = (int)simulationStartingTime + (int)timeIndex * simulationStepTime + floor(simulationStepTime / 2.0);
        sun.setTime(dummytime);

	
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
	  (simulationStepTime / 3600.0);  // sum over all time slices, all year
	
      } // end of day loop simulation

      // Sum up the energy deposited (in KiloWatt-Hours)
      double totalDayEnergyDeposited = 0.0;
      auto hitEnergies = recorder.getSummedHitEnergies();

      for (unsigned int timeIndex = 0; timeIndex < simulationTimeSegments;
	   timeIndex++) {
	for (auto eventHitEnergies : hitEnergies[timeIndex]) {
	  for (auto treeEnergy : eventHitEnergies) {
	    double hitEnergy = (treeEnergy.second / 1000.0) * 
	      (simulationStepTime / 3600.0);
	    totalDayEnergyDeposited += hitEnergy;
	    totalEvaluatedEnergy += hitEnergy;
	    auto wasInserted = yearenergyPerTree.insert({treeEnergy.first, hitEnergy});
	    if (wasInserted.second == false) {
	      yearenergyPerTree[treeEnergy.first] += hitEnergy;
	    }
	  }
	}
      } // end of day loop analysis
      dayEnergySums.push_back(totalDayEnergyDeposited);

      // Don't need to keep old records after analysis performed.
      recorder.reset();
    } // end of year
    
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
    
    std::cout << "Scored Energy [kWh] " << totalEvaluatedEnergy << std::endl;
    
    // Clone the settings/results before moving onto next tree so that they can
    // be saved at the end.
    std::string treeName =
      "tree" + std::to_string(currentForestNumber) + "_Job" + 
      std::to_string(parameterSeed);
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
    clonedTree->setParameter("totalEvaluatedEnergy", totalEvaluatedEnergy);

    std::string leafName = "leaf" + std::to_string(currentForestNumber) + "_Job" +
      std::to_string(parameterSeed);
    LeafConstructionInterface* clonedLeaf =
      (LeafConstructionInterface*)leaf->Clone(leafName.c_str());
    
    // Add to the list that will be exported
    YearlyResult* result = new YearlyResult();

    result->setTree(clonedTree);
    result->setLeaf(clonedLeaf);
    result->setDayTimes(dayTimes);
    result->setEnergyDeposited(dayEnergySums);
    // Sum up the energy deposited (in KiloWatt-Hours), where interpolation
    // is carried out to fill in the gaps.
    double totalEnergyDeposited = result->getEnergyIntegral();
    clonedTree->setParameter("totalIntegratedEnergyDeposit", totalEnergyDeposited);

    exportList.Add(result);

    // Store forest data in TFile
    int treeGridNumber = std::ceil(std::sqrt(treeNumber));
    int counter = 0;
    for (auto& en : yearenergyPerTree) {
      treeID = en.first;
      energyReceived = en.second;
      // next x,y pair
      xID = treeGridNumber-1;
      yID = -xID;
      xID -= treeID % treeGridNumber;
      yID += counter;

      forestdata->Fill();
      if (xID<1) counter++;
    }

    // Move onto next forest
    currentForestNumber++;
    dayTimes.clear();
    dayEnergySums.clear();
    yearenergyPerTree.clear();
  }

  // Job termination
  delete runManager;

  // Write results out to the root file
  resultsFile.cd();
  exportList.Write("testedStructures", TObject::kSingleKey);
  forestdata->Write();

  // Close the root file
  resultsFile.Close();

  // Output some useful information
  std::cout << currentForestNumber << " trees produced in " << treeTrialNumber
            << " trials." << std::endl;

  if (!(treeTrialNumber < maximumTreeTrials)) {
    std::cerr
        << "Not a sufficient number of trials available to satisfy tree demand"
        << std::endl;
    return 1;
  }

  return 0;
}

