/*!
 * @file
 * \brief Application to test the convergence of the simulation by considering
 *        the average energy deposited per unit area with different numbers of
 *        simulated photons.
 *
 *
 */

#include "pvtree/treeSystem/treeFactory.hpp"
#include "pvtree/leafSystem/leafFactory.hpp"
#include "pvtree/geometry/turtle.hpp"
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
#include "pvtree/utils/signalReceiver.hpp"

#include <iostream>
#include <vector>
#include <memory>
#include <exception>
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

/*! \brief Convert set of hits into a point on a graph for a
 *         fixed photon number.
 */
void addHitEfficiencyPoint(TGraphAsymmErrors& graph,
                           std::vector<std::vector<long> > hitCountVectors,
                           int photonNumberPerEvent) {
  // 'Merge' events in the difference hit count vectors to get the
  // total daily efficiency (which is something that should converge).
  if (hitCountVectors.size() == 0) {
    std::cout << "Did not expect there to be no hit count vectors. Probably "
                 "not a good sign!" << std::endl;
    throw new std::exception;
  }

  std::vector<long> summedEventCounts;

  for (unsigned int eventIndex = 0; eventIndex < hitCountVectors[0].size();
       eventIndex++) {
    long sum = 0;
    for (unsigned int countVectorIndex = 0;
         countVectorIndex < hitCountVectors.size(); countVectorIndex++) {
      sum += hitCountVectors[countVectorIndex][eventIndex];
    }

    summedEventCounts.push_back(sum);
  }

  // Effectively now the photon number in the event has gone up by a factor of
  // hitCountVectors.size()
  long photonNumber = photonNumberPerEvent * hitCountVectors.size();

  // Get the mean and standard deviation for the daily number of hits
  double hitSum = 0.0;
  for (unsigned int v = 0; v < summedEventCounts.size(); v++) {
    hitSum += summedEventCounts[v];
  }
  double averageHitNumber = hitSum / summedEventCounts.size();
  double averageEfficiency = averageHitNumber / photonNumber;

  // Calculate standard deviation
  double hitSquareDifferenceSum = 0.0;
  for (unsigned int v = 0; v < summedEventCounts.size(); v++) {
    hitSquareDifferenceSum +=
        std::pow(averageHitNumber - summedEventCounts[v], 2.0);
  }

  double standardDeviation = std::sqrt((1.0 / (summedEventCounts.size() - 1)) *
                                       (hitSquareDifferenceSum));
  double efficiencyStandardDeviation = standardDeviation / photonNumber;

  // Set the point and 'error'.
  int nextPointIndex = graph.GetN();
  graph.SetPoint(nextPointIndex, photonNumber, averageEfficiency);
  graph.SetPointError(nextPointIndex, 0.0, 0.0, efficiencyStandardDeviation,
                      efficiencyStandardDeviation);
}

/*! \brief Add the current energy density point to the graph.
 */
void addEnergyDensityPoint(
    TGraphAsymmErrors& graph,
    std::vector<std::vector<double> > summedEnergyVectors,
    int photonNumberPerEvent, double sensitiveSurfaceArea,
    int simulationStepSize) {
  // 'Merge' events in the difference energy vectors to get the
  // total daily energy (which is something that should converge).
  if (summedEnergyVectors.size() == 0) {
    std::cout << "Did not expect there to be no energy vectors. Probably not a "
                 "good sign!" << std::endl;
    throw new std::exception;
  }

  std::vector<double> sumEnergyDensities;

  for (unsigned int eventIndex = 0; eventIndex < summedEnergyVectors[0].size();
       eventIndex++) {
    // Convert into Watt-seconds whilst summing (currently time steps are all
    // same size)
    double sum = 0.0;
    for (unsigned int energyVectorIndex = 0;
         energyVectorIndex < summedEnergyVectors.size(); energyVectorIndex++) {
      sum += summedEnergyVectors[energyVectorIndex][eventIndex] *
             ((double)simulationStepSize);
    }

    // Also divide by the area to get the energy density and also convert to
    // kWhm^{-2}
    sumEnergyDensities.push_back((sum / sensitiveSurfaceArea) /
                                 (3600.0 * 1000.0));
  }

  // Effectively now the photon number in the event has gone up by a factor of
  // hitCountVectors.size()
  long photonNumber = photonNumberPerEvent * summedEnergyVectors.size();

  // Get the mean and standard deviation for the summed energy
  double energySum = 0.0;
  for (unsigned int v = 0; v < sumEnergyDensities.size(); v++) {
    energySum += sumEnergyDensities[v];
  }
  double averageEnergy = energySum / sumEnergyDensities.size();

  // Calculate standard deviation
  double energySquareDifferenceSum = 0.0;
  for (unsigned int v = 0; v < sumEnergyDensities.size(); v++) {
    energySquareDifferenceSum +=
        std::pow(averageEnergy - sumEnergyDensities[v], 2.0);
  }

  double standardDeviation = std::sqrt((1.0 / (sumEnergyDensities.size() - 1)) *
                                       (energySquareDifferenceSum));

  // Set the point and 'error'.
  int nextPointIndex = graph.GetN();
  graph.SetPoint(nextPointIndex, photonNumber, averageEnergy);
  graph.SetPointError(nextPointIndex, 0.0, 0.0, standardDeviation,
                      standardDeviation);
}

/*! \brief Write out results to root file and create summary plots.
 */
void wrapUp(std::vector<TGraphAsymmErrors> efficiencyGraphs,
            std::vector<TGraphAsymmErrors> energyDensityGraphs,
            std::vector<int> eventPhotonNumbers, int simulationTimeSegments) {
  // Save the raw efficiency graphs
  for (auto& graph : efficiencyGraphs) {
    graph.Write();
  }

  // Save the raw energy density graphs
  for (auto& graph : energyDensityGraphs) {
    graph.Write();
  }

  // Show the efficiency graphs on a set of canvas
  // with some nicer default formatting
  for (auto& graph : efficiencyGraphs) {
    std::string canvasName = std::string(graph.GetName()) + "Canvas";
    TCanvas canvas(canvasName.c_str(), "");

    // Draw band
    graph.Draw("AE3");
    graph.GetXaxis()->SetTitle("N_{photons}");
    graph.GetYaxis()->SetTitle("Efficiency");

    // Draw points
    graph.Draw("SAMEPX");

    // Style
    graph.SetFillColor(kBlue - 8);
    graph.SetLineColor(kBlue - 5);
    graph.SetMarkerColor(kBlue - 5);
    graph.SetMarkerStyle(21);

    canvas.SetLogx(1);
    canvas.Update();
    canvas.Write();
  }

  // Make pretty energy density graphs
  for (auto& graph : energyDensityGraphs) {
    std::string canvasName = std::string(graph.GetName()) + "Canvas";
    TCanvas canvas(canvasName.c_str(), "");

    // Draw band
    graph.Draw("AE3");
    graph.GetXaxis()->SetTitle("N_{photons}");
    graph.GetYaxis()->SetTitle("Surface Energy Density [kWhm^{-2}]");

    // Draw points
    graph.Draw("SAMEPX");

    // Style
    graph.SetFillColor(kBlue - 8);
    graph.SetLineColor(kBlue - 5);
    graph.SetMarkerColor(kBlue - 5);
    graph.SetMarkerStyle(21);

    canvas.SetLogx(1);
    canvas.Update();
    canvas.Write();
  }

  // Fill histograms of relative efficiency-error for diffent photon number
  // trials.
  for (unsigned int p = 0; p < eventPhotonNumbers.size(); p++) {
    // Create a histogram for the relative efficiency error
    std::string histName =
        "relativeEfficiencyErrorPhoton" +
        std::to_string((int)(eventPhotonNumbers[p] * simulationTimeSegments));
    TH1D relativeErrorHist(histName.c_str(),
                           ";#DeltaEfficiency/Efficiency;N_{trees}", 100, 0.0,
                           1.0);

    // Extract the numbers from each graph (tree)
    for (auto& graph : efficiencyGraphs) {
      if (graph.GetY()[p] != 0.0) {
        relativeErrorHist.Fill(graph.GetErrorY(p) / graph.GetY()[p], 1.0);
      }
    }

    relativeErrorHist.Write();
  }

  // Fill histograms of relative surface energy density-error for different
  // photon number trials
  for (unsigned int p = 0; p < eventPhotonNumbers.size(); p++) {
    // Create a histogram for the relative error
    std::string histName =
        "relativeEnergyDensityErrorPhoton" +
        std::to_string((int)(eventPhotonNumbers[p] * simulationTimeSegments));
    TH1D relativeErrorHist(histName.c_str(),
                           ";#DeltaE_{density}/E_density;N_{trees}", 100, 0.0,
                           1.0);

    // Extract the numbers from each graph (tree)
    for (auto& graph : energyDensityGraphs) {
      if (graph.GetY()[p] != 0.0) {
        relativeErrorHist.Fill(graph.GetErrorY(p) / graph.GetY()[p], 1.0);
      }
    }

    relativeErrorHist.Write();
  }
}

void showHelp() {
  std::cout << "convergence help" << std::endl;
  std::cout << "\t -t, --tree <TREE TYPE NAME>" << std::endl;
  std::cout << "\t -l, --leaf <LEAF TYPE NAME>" << std::endl;
  std::cout << "\t --treeNumber <INTEGER>" << std::endl;
  std::cout << "\t --timeSegments <INTEGER>" << std::endl;
  std::cout << "\t --geant4Seed <INTEGER>" << std::endl;
  std::cout << "\t --parameterSeedOffset <INTEGER>" << std::endl;
}

/*! \brief Convergence testing main.
 *
 * Provides an example of how to integrate ROOT analysis objects into a program
 *in
 * addition to validating that the simulation converges with a sufficient number
 *of
 * photons.
 *
 * @param[in] argc Number of command line arguments.
 * @param[in] argv Accepts a number of command line arguments. The GEANT4 seed
 *and the starting offset
 *                 for randomizing tree and leaf parmaeters. These default to 1.
 *The number of trees,
 *                 which defaults to 100. The number of time points which
 *defaults to 10. The type of
 *                 tree and leaf may also be specified.
 *
 */
int main(int argc, char** argv) {
  std::string treeType, leafType;
  unsigned int treeNumber;
  unsigned int simulationTimeSegments;
  int geant4Seed;
  int parameterSeedOffset;

  GetOpt::GetOpt_pp ops(argc, argv);

  // Check for help request
  if (ops >> GetOpt::OptionPresent('h', "help")) {
    showHelp();
    return 0;
  }

  ops >> GetOpt::Option('t', "tree", treeType, "stump");
  ops >> GetOpt::Option('l', "leaf", leafType, "planar");
  ops >> GetOpt::Option("treeNumber", treeNumber, 100u);
  ops >> GetOpt::Option("timeSegments", simulationTimeSegments, 10u);
  ops >> GetOpt::Option("geant4Seed", geant4Seed, 1);
  ops >> GetOpt::Option("parameterSeedOffset", parameterSeedOffset, 1);

  // Report input parameters
  std::cout << "Tree type = " << treeType << std::endl;
  std::cout << "Leaf type = " << leafType << std::endl;
  std::cout << "Using the Geant4 random number seed = " << geant4Seed
            << std::endl;
  std::cout << "Using the parameter random number seed offset = "
            << parameterSeedOffset << std::endl;
  std::cout << "Generating " << treeNumber << " trees." << std::endl;
  std::cout << "Simulating in " << simulationTimeSegments << " time segments."
            << std::endl;

  // Also do not run if other arguments are present
  if (ops.options_remain()) {
    std::cerr << "Oops! Unexpected options." << std::endl;
    showHelp();
    return -1;
  }

  pvtree::loadEnvironment();

  // Require trials with different number of photons in each event
  // and to save time reduce the number of trials with high numbers of
  // photons.
  std::vector<int> eventPhotonNumbers = {10, 20, 50, 100, 200, 1000, 20000};
  std::vector<int> iterationNumbers = {1000, 1000, 1000, 500, 200, 50, 5};

  if (iterationNumbers.size() != eventPhotonNumbers.size()) {
    std::cout
        << "Number of photon numbers and number of iterations don't match."
        << std::endl;
    return -1;
  }

  // Prepare a root file to store the results
  TFile resultsFile("convergence.results.root", "RECREATE");
  std::vector<TGraphAsymmErrors> efficiencyGraphs;
  std::vector<TGraphAsymmErrors> energyDensityGraphs;

  // Setup a signal handler to catch batch job + user terminations
  // so that we can still try to output some of the results.
  // SIGINT == 2 (Ctrl-C on command line)
  // TERM_RUNLIMIT on LSF uses User Defined Signal 2 == 12
  SignalReceiver::instance()->setSignals(
      {2, 12}, [&resultsFile, &efficiencyGraphs, &energyDensityGraphs,
                &eventPhotonNumbers, simulationTimeSegments](int signum) {
        printf("Caught a signal %d\n", signum);

        // Run the standard plot creation routines with
        // whatever simulation results finished in time.
        wrapUp(efficiencyGraphs, energyDensityGraphs, eventPhotonNumbers,
               simulationTimeSegments);

        // Close the root file
        resultsFile.Close();

        // Terminate program
        exit(signum);
      });

  // Choose the Random engine
  G4Random::setTheEngine(new CLHEP::RanecuEngine);
  G4Random::setTheSeed(geant4Seed);

  // Get the device location details
  LocationDetails deviceLocation("location.cfg");

  // Set the altitude of the spectrum factory using location details
  SpectrumFactory::instance()->setAltitude(deviceLocation.getAltitude());

  // Prepare the climate factory with the default configuration
  ClimateFactory::instance()->setConfigurationFile("default.cfg");
  ClimateFactory::instance()->setDeviceLocation(deviceLocation);

  // Prepare initial conditions for test trunk and leaves
  auto tree = TreeFactory::instance()->getTree(treeType);
  auto leaf = LeafFactory::instance()->getLeaf(leafType);

  // Define the sun setting, just an arbitrary date for now
  // Perform the simulation between the sunrise and sunset.
  Sun sun(deviceLocation);
  sun.setDate(190, 2014);
  int simulationStartingTime = sun.getSunriseTime() * 60;  // s
  int simulationEndingTime = sun.getSunsetTime() * 60;  // s,
  int simulationStepTime =
      (double)(simulationEndingTime - simulationStartingTime) /
      simulationTimeSegments;

  std::cout << "Simulation time considered between " << simulationStartingTime
            << "(s) and " << simulationEndingTime << "(s)." << std::endl;

  // Set the default materials to be used
  MaterialFactory::instance()->addConfigurationFile("defaults-tree.cfg");

  // Construct the default run manager
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
      [&eventPhotonNumbers, &sun ]() -> G4VUserPrimaryGeneratorAction *
      { return new PrimaryGeneratorAction(eventPhotonNumbers[0], &sun); }));

  // Initialize G4 kernel
  runManager->Initialize();

  // Repeat for a number of trees
  for (unsigned int x = 0; x < treeNumber; x++) {
    std::cout << "Considering tree " << x << std::endl;

    // Create a new efficiency graph for this tree
    TGraphAsymmErrors effGraph;
    std::string graphName = "efficiencyCheckForTree" + std::to_string(x);
    effGraph.SetName(graphName.c_str());

    // Create an energy density graph for this tree
    TGraphAsymmErrors densityGraph;
    graphName = "densityForTree" + std::to_string(x);
    densityGraph.SetName(graphName.c_str());

    // Allow the geometry to be rebuilt with new settings
    tree->randomizeParameters(x + parameterSeedOffset);
    leaf->randomizeParameters(x + parameterSeedOffset);
    detector->resetGeometry(tree, leaf);

    // Re-initialize the detector geometry
    G4bool destroyFirst;
    runManager->ReinitializeGeometry(destroyFirst = true);

    // Simulate with a range of different photon numbers in different events
    for (unsigned int p = 0; p < eventPhotonNumbers.size(); p++) {
      // Set photon number of primary generation
      ((PrimaryGeneratorAction*)runManager->GetUserPrimaryGeneratorAction())
          ->SetPhotonNumber(eventPhotonNumbers[p]);

      int numberOfEvent = iterationNumbers[p];

      // Simulate at all time points with the same number of events...
      for (unsigned int timeIndex = 0; timeIndex < simulationTimeSegments;
           timeIndex++) {
        // Set the time to the mid-point of the time segment
        sun.setTime((int)(simulationStartingTime +
                          timeIndex * simulationStepTime +
                          simulationStepTime / 2.0));

        // Run simulation
        runManager->BeamOn(numberOfEvent);
      }

      // Consider all simulation time points at once.
      //! \todo Investigate number of time points required for converegence :)
      addHitEfficiencyPoint(effGraph, recorder.getHitCounts(),
                            eventPhotonNumbers[p]);
      addEnergyDensityPoint(
          densityGraph, recorder.getSummedHitEnergies(), eventPhotonNumbers[p],
          detector->getSensitiveSurfaceArea(), simulationStepTime);

      // Don't need to keep old records after analysis performed.
      recorder.reset();
    }

    // Save the efficiency graph to the export vector and ensure y-axis range is
    // always 0 to 1
    effGraph.SetMaximum(1.0);
    effGraph.SetMinimum(0.0);
    efficiencyGraphs.push_back(effGraph);

    // Also record the energy density
    energyDensityGraphs.push_back(densityGraph);
  }
  // Job termination
  delete runManager;

  // Write out results and some summary plots
  wrapUp(efficiencyGraphs, energyDensityGraphs, eventPhotonNumbers,
         simulationTimeSegments);

  // Close the root file
  resultsFile.Close();

  return 0;
}
