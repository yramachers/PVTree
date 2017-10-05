/*!
 * @file
 * \brief Application to investigate the distribution of incident light
 *        across a forest over the period of one day.
 *
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
#include "TGraph2D.h"
#include "TRandom.h"
#include "TStyle.h"

// turn the warnings back on
#pragma GCC diagnostic pop

void showHelp() {
  std::cout << "forestScan help" << std::endl;
  std::cout << "\t -t, --tree <TREE TYPE NAME>" << std::endl;
  std::cout << "\t -l, --leaf <LEAF TYPE NAME>" << std::endl;
  std::cout << "\t --treeNumber <INTEGER>" << std::endl;
  std::cout << "\t --timeSegments <INTEGER>" << std::endl;
  std::cout << "\t --photonNumber <INTEGER>" << std::endl;
  std::cout << "\t --geant4Seed <INTEGER>" << std::endl;
  std::cout << "\t --inputTreeFile <ROOT FILENAME>" << std::endl;
}

/*! 
 *
 * @param[in] argc Number of command line arguments.
 * @param[in] argv Accepts a number of command line arguments.
 *                 The GEANT4 seed, which defaults to 1.
 *                 The number of trees, which defaults to 1. 
 *                 The number of time points, which defaults to 50. 
 *                 The number of photons to generate in each time segment, 
 *            which defaults to 500. 
 *                 The type of tree and leaf may also be specified.
 *
 */
int main(int argc, char** argv) {
  std::string treeType, leafType;
  unsigned int treeNumber;
  unsigned int simulationTimeSegments;
  unsigned int photonNumberPerTimeSegment;
  int geant4Seed;
  std::string inputTreeFileName;
  bool singleTreeRunning = false;

  GetOpt::GetOpt_pp ops(argc, argv);

  // Check for help request
  if (ops >> GetOpt::OptionPresent('h', "help")) {
    showHelp();
    return 0;
  }

  ops >> GetOpt::Option('t', "tree", treeType, "monopodial");
  ops >> GetOpt::Option('l', "leaf", leafType, "cordate");
  ops >> GetOpt::Option("treeNumber", treeNumber, 1u);
  ops >> GetOpt::Option("timeSegments", simulationTimeSegments, 50u);
  ops >> GetOpt::Option("photonNumber", photonNumberPerTimeSegment, 500u);
  ops >> GetOpt::Option("geant4Seed", geant4Seed, 1);
  ops >> GetOpt::Option("inputTreeFile", inputTreeFileName, "");

  // Report input parameters
  if (inputTreeFileName != "") {
    std::cout << "Just using selected tree from " << inputTreeFileName
              << std::endl;
    singleTreeRunning = true;
  } else {
    std::cout << "Tree type = " << treeType << std::endl;
    std::cout << "Leaf type = " << leafType << std::endl;
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

  // Prepare a root file to store the results
  TFile resultsFile("forestScan.results.root", "RECREATE");


  double totalNormal;
  double totalDiffuse;
  double totalInitial = 0.0;

  // Simulate at all time points with the same number of events...
  totalInitial = 0.0;
  int dummytime = 0;
  for (unsigned int timeIndex = 0; timeIndex < simulationTimeSegments;
        timeIndex++) {
    // Set the time to the mid-point of the time segment
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
        (simulationStepTime / 3600.0);  // sum over all time slices
  }

  // Get the total surface area which is "sensitive" from current tested
  // detector.
  double sensitiveArea = detector->getSensitiveSurfaceArea();

  // Get the number of leaves
  int numberOfLeaves = detector->getNumberOfLeaves();
  //std::cout << "Number of leaves: " << numberOfLeaves << std::endl;
  int numberOfRejectedLeaves = detector->getNumberOfRejectedLeaves();


  // Sum up the energy deposited (in KiloWatt-Hours)
  double totalEnergyDeposited = 0.0;
  std::map<unsigned int, double> energyPerTree;
  auto hitEnergies = recorder.getSummedHitEnergies();

  for (unsigned int timeIndex = 0; timeIndex < simulationTimeSegments;
       timeIndex++) {
    for (auto eventHitEnergies : hitEnergies[timeIndex]) {
      for (auto treeEnergy : eventHitEnergies) {
        double hitEnergy = (treeEnergy.second / 1000.0) * 
                           (simulationStepTime / 3600.0);
        totalEnergyDeposited += hitEnergy;
        auto wasInserted = energyPerTree.insert({treeEnergy.first, hitEnergy});
        if (wasInserted.second == false) {
          energyPerTree[treeEnergy.first] += hitEnergy;
        }
      }
    }
  }
  // Don't need to keep old records after analysis performed.
  recorder.reset();

  std::cout << "Scored Energy [kWh] " << totalEnergyDeposited << std::endl;
  std::cout << "from Initial [kWh/m^2] " << totalInitial << std::endl;
  std::cout << "on Area [m^2] " << sensitiveArea << std::endl;
  std::cout << numberOfLeaves << " leaves constructed, out of " 
            << numberOfRejectedLeaves <<std::endl;

  // Draw energy per tree in ROOT 
  unsigned int treeGridNumber = std::ceil(std::sqrt(treeNumber));
  TCanvas *c1 = new TCanvas("c1","c1");
  TGraph2D *dt = new TGraph2D();
  dt->SetTitle("Scored Energy [kwh] across the forest");
  Double_t x, y, z;
  Int_t P = treeGridNumber;
  Int_t np = treeNumber*200;
  TRandom *r = new TRandom();
  // Randomly generate points and find the energy incident on the tree at that
  // location.
  for (Int_t N=0; N<np; N++) {
    x = P*(r->Rndm(N));
    y = P*(r->Rndm(N));
    unsigned int indexNumber = int(x)+treeGridNumber*int(y);
    z = energyPerTree[indexNumber];
    dt->SetPoint(N,x,y,z);
  }
  dt->Draw("colz");
  resultsFile.WriteTObject(c1);


  // Job termination
  delete runManager;

  // Close the root file
  resultsFile.Close();

  return 0;
}

