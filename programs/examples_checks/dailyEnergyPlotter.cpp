/*!
 * @file
 * \brief Application to plot the energy collected over a period of
 *        a day.
 *
 * Trees can either be generated at random or a specific tree can
 * be passed in via a root file.
 */

#include "pvtree/treeSystem/treeFactory.hpp"
#include "pvtree/leafSystem/leafFactory.hpp"
#include "pvtree/full/detectorConstruction.hpp"
#include "pvtree/full/actionInitialization.hpp"
#include "pvtree/full/primaryGeneratorAction.hpp"
#include "pvtree/full/opticalPhysicsList.hpp"
#include "pvtree/full/recorders/convergenceRecorder.hpp"
#include "pvtree/full/solarSimulation/sun.hpp"
#include "pvtree/full/material/materialFactory.hpp"
#include "pvtree/utils/getopt_pp.h"
#include "pvtree/utils/resource.hpp"
#include "pvtree/climate/climateFactory.hpp"
#include "pvtree/location/locationDetails.hpp"
#include "pvtree/full/solarSimulation/spectrumFactory.hpp"
#include "pvtree/analysis/yearlyResult.hpp"

#include <iostream>
#include <vector>
#include <memory>
#include <chrono>
#include <sstream>

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
  std::cout << "dailyEnergyPlotter help" << std::endl;
  std::cout << "\t -t, --tree <TREE TYPE NAME> :\t default 'stump'"
            << std::endl;
  std::cout << "\t -l, --leaf <LEAF TYPE NAME> :\t default 'planar'"
            << std::endl;
  std::cout << "\t --treeNumber <INTEGER> :\t default 3" << std::endl;
  std::cout << "\t --timeSegments <INTEGER> :\t default 25" << std::endl;
  std::cout << "\t --photonNumber <INTEGER> :\t default 100000" << std::endl;
  std::cout << "\t --geant4Seed <INTEGER> :\t default 1" << std::endl;
  std::cout << "\t --parameterSeedOffset <INTEGER> :\t default 1" << std::endl;
  std::cout << "\t --inputTreeFile <ROOT FILENAME> :\t default ''" << std::endl;
  std::cout << "\t --day <INTEGER> :\t default 190" << std::endl;
  std::cout << "\t --outputFileName <ROOT FILENAME> : \t default "
               "'dailyEnergyPlotter.results.root'" << std::endl;
}

void createSummaryCanvas(std::vector<TGraphAsymmErrors>& graphs,
                         std::string canvasName, std::string xAxisTitle,
                         std::string yAxisTitle) {
  if (graphs.size() < 1) {
    // Don't do anything.
    return;
  }

  TCanvas canvas(canvasName.c_str(), "");

  // Draw first graph
  graphs[0].Draw("AL");
  graphs[0].GetXaxis()->SetTitle(xAxisTitle.c_str());
  graphs[0].GetYaxis()->SetTitle(yAxisTitle.c_str());

  // Set time axis format
  graphs[0].GetXaxis()->SetTimeDisplay(1);
  graphs[0].GetXaxis()->SetTimeFormat("%H:%M");

  // Repeat for other graphs
  for (unsigned int g = 1; g < graphs.size(); g++) {
    graphs[g].Draw("SAMEL");
  }

  // Save to disk
  canvas.Update();
  canvas.Write();
}

/*! \brief Time binned energy plotter main.
 *
 * Provides an example of how to perform a random search with output simply
 * being the energy as a function of time over the period of a day.
 *
 * @param[in] argc Number of command line arguments.
 * @param[in] argv See dailyEnergyPlotter --help (showHelp) for available
 *arguments.
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
  unsigned int dayNumber;
  std::string outputFileName;

  GetOpt::GetOpt_pp ops(argc, argv);

  // Check for help request
  if (ops >> GetOpt::OptionPresent('h', "help")) {
    showHelp();
    return 0;
  }

  ops >> GetOpt::Option('t', "tree", treeType, "stump");
  ops >> GetOpt::Option('l', "leaf", leafType, "planar");
  ops >> GetOpt::Option("treeNumber", treeNumber, 3u);
  ops >> GetOpt::Option("timeSegments", simulationTimeSegments, 25u);
  ops >> GetOpt::Option("photonNumber", photonNumberPerTimeSegment, 100000u);
  ops >> GetOpt::Option("geant4Seed", geant4Seed, 1);
  ops >> GetOpt::Option("parameterSeedOffset", parameterSeedOffset, 1);
  ops >> GetOpt::Option("inputTreeFile", inputTreeFileName, "");
  ops >> GetOpt::Option("day", dayNumber, 190u);
  ops >> GetOpt::Option("outputFileName", outputFileName,
                        "dailyEnergyPlotter.results.root");

  // Report input parameters
  if (inputTreeFileName != "") {
    std::cout << "Just using best tree from " << inputTreeFileName << std::endl;
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
  std::cout << "Recording results in " << outputFileName << std::endl;

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
  TreeConstructionInterface* bestT{nullptr};
  LeafConstructionInterface* bestL{nullptr};

  if (!singleTreeRunning) {
    tree = TreeFactory::instance()->getTree(treeType);
    leaf = LeafFactory::instance()->getLeaf(leafType);
  } else {
    TFile inputTreeFile(inputTreeFileName.c_str(), "READ");
    TList* structureList = (TList*)inputTreeFile.Get("testedStructures");
    TIter structureListIterator(structureList);

    if (structureList->GetSize() == 0) {
      std::cout << "There are no trees to consider." << std::endl;
      return 1;
    }
    double area;
    double energy;
    double eff;
    double besteff = 0.0;
    while (YearlyResult* currentStructure =
               (YearlyResult*)structureListIterator()) {
      TreeConstructionInterface* clonedT = currentStructure->getTree();
      LeafConstructionInterface* clonedL = currentStructure->getLeaf();
      ;
      area = clonedT->getDoubleParameter("sensitiveArea");
      energy = clonedT->getDoubleParameter("totalEnergy");
      eff = energy / area;
      if (eff > besteff) {  // book best tree
        bestT = clonedT;
        bestL = clonedL;
        besteff = eff;
        std::cout << "RETRIEVE: got total energy = " << energy
                  << " eff: " << eff << std::endl;
      }
    }
    //     tree = std::shared_ptr<TreeConstructionInterface>(
    //     (TreeConstructionInterface*)inputTreeFile.FindObjectAny("selectedTree")
    //     );
    //     leaf = std::shared_ptr<LeafConstructionInterface>(
    //     (LeafConstructionInterface*)inputTreeFile.FindObjectAny("selectedLeaf")
    //     );
    tree = std::shared_ptr<TreeConstructionInterface>(bestT);
    leaf = std::shared_ptr<LeafConstructionInterface>(bestL);

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
  sun.setDate(dayNumber, 2014);
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

  // Store a TGraph for each tree
  std::vector<TGraphAsymmErrors> energyGraphs;
  std::vector<TGraphAsymmErrors> normalizedEnergyGraphs;
  std::vector<TGraphAsymmErrors> energyDensityGraphs;

  // Repeat for a number of trees
  for (unsigned int x = 0; x < treeNumber; x++) {
    TGraphAsymmErrors currentEnergyGraph;
    std::string graphName = "energyGraph_tree" + std::to_string(x);
    currentEnergyGraph.SetName(graphName.c_str());
    currentEnergyGraph.SetTitle("");
    currentEnergyGraph.GetXaxis()->SetTitle("Time since midnight [s]");
    currentEnergyGraph.GetYaxis()->SetTitle("Energy [kWh]");

    TGraphAsymmErrors currentNormalizedEnergyGraph;
    graphName = "normalizedEnergyGraph_tree" + std::to_string(x);
    currentNormalizedEnergyGraph.SetName(graphName.c_str());
    currentNormalizedEnergyGraph.SetTitle("");
    currentNormalizedEnergyGraph.GetXaxis()->SetTitle(
        "Time since midnight [s]");
    currentNormalizedEnergyGraph.GetYaxis()->SetTitle("Fractional Energy");

    TGraphAsymmErrors currentEnergyDensityGraph;
    graphName = "energyDensityGraph_tree" + std::to_string(x);
    currentEnergyDensityGraph.SetName(graphName.c_str());
    currentEnergyDensityGraph.SetTitle("");
    currentEnergyDensityGraph.GetXaxis()->SetTitle("Time since midnight [s]");
    currentEnergyDensityGraph.GetYaxis()->SetTitle(
        "Energy density [kWhm^{-2}]");

    if (!singleTreeRunning) {
      // Allow the geometry to be rebuilt with new settings
      tree->randomizeParameters(x + parameterSeedOffset);
      leaf->randomizeParameters(x + parameterSeedOffset);

      detector->resetGeometry(tree, leaf);

      // Re-initialize the detector geometry
      //      runManager->GeometryHasBeenModified();
      runManager->ReinitializeGeometry(true, false);         // clean up
      runManager->BeamOn(0); // fake start to build geometry
  //      G4bool destroyFirst;
  //      runManager->ReinitializeGeometry(destroyFirst = true);
  //      runManager->Initialize();
    }

    if (x % 50 == 0) {
      std::cout << "Considering tree " << x << std::endl;
      tree->print();
      leaf->print();
    }

    // Simulate at all time points with the same number of events...
    for (unsigned int timeIndex = 0; timeIndex < simulationTimeSegments;
         timeIndex++) {
      // Set the time to the mid-point of the time segment
      sun.setTime((int)(simulationStartingTime +
                        timeIndex * simulationStepTime +
                        simulationStepTime / 2.0));

      // Run simulation with a single event per time point
      G4int eventNumber = 1;
      runManager->BeamOn(eventNumber);
    }

    // Get the sensitive area of tree
    double currentSensitiveArea = detector->getSensitiveSurfaceArea();

    // Sum up the energy deposited (in KiloWatt hour)
    std::vector<std::vector<double> > hitEnergies =
        recorder.getSummedHitEnergies();

    // Grab the total energy sum firstly for normalization!
    double totalEnergy = 0.0;

    for (unsigned int timeIndex = 0; timeIndex < simulationTimeSegments;
         timeIndex++) {
      // Sum up the energy deposited in a 'run' (should just be a single event
      // per run).
      double totalRunEnergy = 0.0;  // kWh
      for (double eventHitEnergy : hitEnergies[timeIndex]) {
        totalRunEnergy +=
            (eventHitEnergy / 1000.0) * (simulationStepTime / 3600.0);
      }

      totalEnergy += totalRunEnergy;
    }
    std::cout << "SIM: got total energy = " << totalEnergy << std::endl;
    std::cout << "SIM: sensitive area = " << currentSensitiveArea << std::endl;

    // Now fill the graph
    for (unsigned int timeIndex = 0; timeIndex < simulationTimeSegments;
         timeIndex++) {
      // Sum up the energy deposited in a 'run' (should just be a single event
      // per run).
      double totalRunEnergy = 0.0;  // kWh
      for (double eventHitEnergy : hitEnergies[timeIndex]) {
        totalRunEnergy +=
            (eventHitEnergy / 1000.0) * (simulationStepTime / 3600.0);
      }

      // Add the point to the graph
      int currentTime =
          (int)(simulationStartingTime + timeIndex * simulationStepTime +
                simulationStepTime / 2.0);
      int nextPointIndex = currentEnergyGraph.GetN();

      currentEnergyGraph.SetPoint(nextPointIndex, currentTime, totalRunEnergy);
      currentNormalizedEnergyGraph.SetPoint(nextPointIndex, currentTime,
                                            totalRunEnergy / totalEnergy);
      currentEnergyDensityGraph.SetPoint(nextPointIndex, currentTime,
                                         totalRunEnergy / currentSensitiveArea);
    }

    // Don't need to keep old records after analysis performed.
    recorder.reset();

    // Store for later writing.
    energyGraphs.push_back(currentEnergyGraph);
    normalizedEnergyGraphs.push_back(currentNormalizedEnergyGraph);
    energyDensityGraphs.push_back(currentEnergyDensityGraph);
  }

  // Job termination
  delete runManager;

  // Prepare a root file to store the results
  TFile resultsFile(outputFileName.c_str(), "RECREATE");

  // Write results out to the root file
  for (auto& graph : energyGraphs) {
    graph.Write();
  }
  for (auto& graph : normalizedEnergyGraphs) {
    graph.Write();
  }
  for (auto& graph : energyDensityGraphs) {
    graph.Write();
  }

  // Make a canvas combining all the plots in one summary graphic
  createSummaryCanvas(energyGraphs, "energySummaryCanvas", "Time [H:M]",
                      "Energy [kWh]");
  createSummaryCanvas(normalizedEnergyGraphs, "normalizedSummaryCanvas",
                      "Time [H:M]", "Fractional Energy");
  createSummaryCanvas(energyDensityGraphs, "energyDensityCanvas", "Time [H:M]",
                      "Energy density [kWhm^{-2}]");

  // Close the root file
  resultsFile.Close();

  return 0;
}
