/*!
 * @file
 * \brief Application to plot the energy collected over a period of
 *        a year.
 *
 * Trees can either be generated at random or a specific tree can
 * be passed in via a root file.
 */

#include "pvtree/treeSystem/treeFactory.hpp"
#include "pvtree/leafSystem/leafFactory.hpp"
#include "pvtree/full/recorders/convergenceRecorder.hpp"
#include "pvtree/analysis/yearlyResult.hpp"
#include "pvtree/utils/getopt_pp.h"
#include "pvtree/utils/resource.hpp"

#include <iostream>
#include <vector>
#include <memory>
#include <chrono>
#include <sstream>
#include <cmath>
#include <tuple>
#include <algorithm>

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
  std::cout << "yearlyEnergyPlotter help" << std::endl;
  //   std::cout << "\t --treeNumber <INTEGER> :\t default 10" << std::endl;
  std::cout << "\t --inputTreeFile <ROOT FILENAME> :\t default ''" << std::endl;
  //   std::cout << "\t --minimumSensitveArea <DOUBLE> [m^2] :\t default 0.001"
  //   << std::endl;
  std::cout << "\t --outputFileName <ROOT FILENAME> : \t default "
               "'yearlyEnergyPlotter.results.root'" << std::endl;
}

double offsetToRootTime(time_t currentTime) {
  // ROOT defines time as starting from 01/01/1995, so I need to move the time
  // to there
  struct tm calendarTime;
  calendarTime.tm_sec = 0;
  calendarTime.tm_min = 0;
  calendarTime.tm_hour = 1;
  calendarTime.tm_mday = 1;
  calendarTime.tm_mon = 0;
  calendarTime.tm_year = 95;
  calendarTime.tm_isdst = 1;

  time_t rootStartTime = mktime(&calendarTime);

  // Convert to ROOT epoch time
  return difftime(currentTime, rootStartTime);
}

void createSummaryCanvas(TGraphAsymmErrors& graphs, std::string canvasName,
                         std::string xAxisTitle, std::string yAxisTitle) {
  //   if (graphs.size() < 1){
  // Don't do anything.
  //     return;
  //   }

  TCanvas canvas(canvasName.c_str(), "");

  // Draw first graph
  graphs.Draw("AL");
  graphs.GetXaxis()->SetTitle(xAxisTitle.c_str());
  graphs.GetYaxis()->SetTitle(yAxisTitle.c_str());
  graphs.SetLineColorAlpha(kRed - 2, 0.01);
  graphs.GetXaxis()->SetTimeDisplay(1);
  graphs.GetXaxis()->SetTimeFormat("%d/%m/%Y");

  // Repeat for other graphs
  //   for (unsigned int g = 1; g < graphs.size(); g++){
  //     graphs[g].SetLineColorAlpha(kRed-2, 0.01);
  //     graphs[g].Draw("SAMEL");
  //   }

  // Save to disk
  canvas.Update();
  canvas.Write();
}

void createAverageGraph(TGraphAsymmErrors& resultGraph,
                        std::vector<TGraphAsymmErrors> inputGraphs) {
  if (inputGraphs.size() == 0) {
    // Nothing to do
    return;
  }

  // Initially just check all the input graphs have the same number of points
  int numberOfPoints = inputGraphs[0].GetN();

  for (auto& graph : inputGraphs) {
    if (numberOfPoints != graph.GetN()) {
      // Inconsistency
      std::cout << "Different number of points present in plots for averaging."
                << std::endl;
      return;
    }
  }

  // For each point on each graph find the average and standard deviation
  for (int p = 0; p < numberOfPoints; p++) {
    std::vector<double> yValues;

    for (auto& graph : inputGraphs) {
      yValues.push_back(graph.GetY()[p]);
    }

    // Calculate the average
    double averageY = 0.0;
    for (auto& val : yValues) {
      averageY += val;
    }
    averageY /= inputGraphs.size();

    // Calculate the standard deviation
    double standardDeviationY = 0.0;
    double sumSquaredDifference = 0.0;

    for (auto& val : yValues) {
      sumSquaredDifference += std::pow(val - averageY, 2.0);
    }
    standardDeviationY = std::sqrt(sumSquaredDifference / (yValues.size() - 1));

    // Set the point
    int nextPointIndex = resultGraph.GetN();

    resultGraph.SetPoint(nextPointIndex, inputGraphs[0].GetX()[p], averageY);

    resultGraph.SetPointEYhigh(nextPointIndex, standardDeviationY);
    resultGraph.SetPointEYlow(nextPointIndex, standardDeviationY);

    resultGraph.SetPointEXhigh(nextPointIndex, inputGraphs[0].GetEXhigh()[p]);
    resultGraph.SetPointEXlow(nextPointIndex, inputGraphs[0].GetEXlow()[p]);
  }
}

time_t timeOfMonthStart(int month, int year) {
  struct tm calendarTime;
  calendarTime.tm_sec = 0;
  calendarTime.tm_min = 0;
  calendarTime.tm_hour = 12;
  calendarTime.tm_mday = 1;
  calendarTime.tm_mon = month;
  calendarTime.tm_year = year;
  calendarTime.tm_isdst = 1;
  return mktime(&calendarTime);
}

time_t timeOfMonthEnd(int month, int year) {
  // Find next month
  month++;

  // Handle year wrapping
  if (month == 12) {
    year++;
    month = 0;
  }

  struct tm calendarTime;
  calendarTime.tm_sec = 0;
  calendarTime.tm_min = 0;
  calendarTime.tm_hour = 12;
  calendarTime.tm_mday = 1;
  calendarTime.tm_mon = month;
  calendarTime.tm_year = year;
  calendarTime.tm_isdst = 1;
  return mktime(&calendarTime);
}

void fillGraphWithIntegratedMonth(TGraphAsymmErrors& graph,
                                  YearlyResult* structure) {
  // Just handling the energy density at the moment, so scale by the sensitive
  // area
  double sensitiveArea =
      structure->getTree()->getDoubleParameter("sensitiveArea");

  // Build a list of months to fill
  std::vector<time_t> dayTimes = structure->getDayTimes();

  std::vector<std::tuple<int, int>> monthYearEntries;
  for (auto& day : dayTimes) {
    struct tm* calendarTime = gmtime(&day);

    int month = calendarTime->tm_mon;
    int year = calendarTime->tm_year;

    // Add to month year entries if not already present
    auto nextMonthYear = std::make_tuple(month, year);

    if (std::find(begin(monthYearEntries), end(monthYearEntries),
                  nextMonthYear) == monthYearEntries.end()) {
      monthYearEntries.push_back(nextMonthYear);
    }
  }

  if (monthYearEntries.size() == 0) {
    return;
  }

  // Fill any gaps in the month vector
  std::vector<std::tuple<int, int>> filledMonthYearEntries;
  filledMonthYearEntries.push_back(monthYearEntries[0]);

  while (filledMonthYearEntries.back() != monthYearEntries.back()) {
    int currentMonth = std::get<0>(filledMonthYearEntries.back());
    int currentYear = std::get<1>(filledMonthYearEntries.back());

    if (currentMonth == 11) {
      filledMonthYearEntries.push_back(std::make_tuple(0, currentYear + 1));
      continue;
    }

    filledMonthYearEntries.push_back(
        std::make_tuple(currentMonth + 1, currentYear));

    // Avoid memory filling error
    if (filledMonthYearEntries.size() > 1000) {
      throw std::string("Error in creating month-year list (did not stop!).");
    }
  }

  // Check that there are simulated points before and after months
  std::vector<std::tuple<int, int>> selectedMonthYearEntries;

  for (auto& monthYearTuple : filledMonthYearEntries) {
    time_t monthIntegrationStart = timeOfMonthStart(
        std::get<0>(monthYearTuple), std::get<1>(monthYearTuple));
    time_t monthIntegrationEnd = timeOfMonthEnd(std::get<0>(monthYearTuple),
                                                std::get<1>(monthYearTuple));

    // Allow 12 hour leeway
    if (monthIntegrationStart < dayTimes[0] - (60 * 60 * 12)) {
      continue;
    }
    if (monthIntegrationEnd > dayTimes.back() + (60 * 60 * 12)) {
      continue;
    }

    selectedMonthYearEntries.push_back(monthYearTuple);
  }

  //! \todo Check for Large gaps and handle them differently...

  // Test print out all the month year pairs
  /*
  for ( auto& monthYearTuple : filledMonthYearEntries ){
    std::cout << "FILL month: " << std::get<0>(monthYearTuple) << " year: " <<
  std::get<1>(monthYearTuple) << std::endl;
  }

  for ( auto& monthYearTuple : selectedMonthYearEntries ){
    std::cout << "USING month: " << std::get<0>(monthYearTuple) << " year: " <<
  std::get<1>(monthYearTuple) << std::endl;
  }

  throw;
  */

  // Apply integration for each month and then add a new point
  for (auto& monthYearTuple : selectedMonthYearEntries) {
    // Build start and end times for month being integrated
    time_t monthIntegrationStart = timeOfMonthStart(
        std::get<0>(monthYearTuple), std::get<1>(monthYearTuple));
    time_t monthIntegrationEnd = timeOfMonthEnd(std::get<0>(monthYearTuple),
                                                std::get<1>(monthYearTuple));

    double energyIntegral = structure->getEnergyIntegral(monthIntegrationStart,
                                                         monthIntegrationEnd);

    // Add point to the graph
    long rootStartTime = offsetToRootTime(monthIntegrationStart);
    long rootEndTime = offsetToRootTime(monthIntegrationEnd);

    long centreTime = rootStartTime + (rootEndTime - rootStartTime) / 2.0;

    int nextPointIndex = graph.GetN();
    graph.SetPoint(nextPointIndex, centreTime, energyIntegral / sensitiveArea);
    graph.SetPointEXlow(nextPointIndex, centreTime - rootStartTime);
    graph.SetPointEXhigh(nextPointIndex, rootEndTime - centreTime);
  }
}

void fillGraphWithIntegratedYear(TGraphAsymmErrors& graph,
                                 YearlyResult* structure) {
  // Just handling the energy density at the moment, so scale by the sensitive
  // area
  double sensitiveArea =
      structure->getTree()->getDoubleParameter("sensitiveArea");

  // Build a list of years to fill
  std::vector<time_t> dayTimes = structure->getDayTimes();

  std::vector<int> yearEntries;
  for (auto& day : dayTimes) {
    struct tm* calendarTime = gmtime(&day);
    int year = calendarTime->tm_year;

    if (std::find(begin(yearEntries), end(yearEntries), year) ==
        yearEntries.end()) {
      yearEntries.push_back(year);
    }
  }

  if (yearEntries.size() == 0) {
    return;
  }

  // Fill any gaps in the year vector
  std::vector<int> filledYearEntries;
  filledYearEntries.push_back(yearEntries[0]);

  while (filledYearEntries.back() != yearEntries.back()) {
    int currentYear = filledYearEntries.back();
    filledYearEntries.push_back(currentYear + 1);

    // Avoid memory filling error
    if (filledYearEntries.size() > 1000) {
      throw std::string("Error in creating year list (did not stop!).");
    }
  }

  // Check that there are simulated points before and after months
  std::vector<int> selectedYearEntries;

  for (auto& year : filledYearEntries) {
    time_t yearIntegrationStart = timeOfMonthStart(0, year);
    time_t yearIntegrationEnd = timeOfMonthStart(0, year + 1);

    // Allow 12 hour leeway
    if (yearIntegrationStart < dayTimes[0] - (60 * 60 * 12)) {
      continue;
    }
    if (yearIntegrationEnd > dayTimes.back() + (60 * 60 * 12)) {
      continue;
    }

    selectedYearEntries.push_back(year);
  }

  //! \todo Check for Large gaps and handle them differently...

  // Test print out all the month year pairs
  /*
  for( int year : selectedYearEntries ){
    std::cout << "Using Year : " << year << std::endl;
  }

  throw;
  */

  // Apply integration for each month and then add a new point
  for (auto& year : selectedYearEntries) {
    time_t yearIntegrationStart = timeOfMonthStart(0, year);
    time_t yearIntegrationEnd = timeOfMonthStart(0, year + 1);

    double energyIntegral =
        structure->getEnergyIntegral(yearIntegrationStart, yearIntegrationEnd);

    // Add point to the graph
    long rootStartTime = offsetToRootTime(yearIntegrationStart);
    long rootEndTime = offsetToRootTime(yearIntegrationEnd);

    long centreTime = rootStartTime + (rootEndTime - rootStartTime) / 2.0;

    int nextPointIndex = graph.GetN();
    graph.SetPoint(nextPointIndex, centreTime, energyIntegral / sensitiveArea);
    graph.SetPointEXlow(nextPointIndex, centreTime - rootStartTime);
    graph.SetPointEXhigh(nextPointIndex, rootEndTime - centreTime);
  }
}

void fillGraphs(YearlyResult* currentStructure, std::string outputFileName) {
  // Iterate through the scan results list
  //   TIter scanIterator(scanResults);

  // Store a TGraph for each tree
  //   std::vector<TGraphAsymmErrors> energyGraphs;
  //   std::vector<TGraphAsymmErrors> normalizedEnergyGraphs;
  //   std::vector<TGraphAsymmErrors> energyDensityGraphs;

  // For monthly integrations
  std::vector<TGraphAsymmErrors> monthlyEnergyDensityGraphs;

  // For yearly integrations
  std::vector<TGraphAsymmErrors> yearlyEnergyDensityGraphs;

  //   for (int s=0; s<scanResults->GetSize(); s++ ){

  //     if (s > treeNumber) {
  //       break;
  //     }

  TGraphAsymmErrors currentEnergyGraph;
  std::string graphName = "energyGraph_tree";
  currentEnergyGraph.SetName(graphName.c_str());
  currentEnergyGraph.SetTitle("");
  currentEnergyGraph.GetXaxis()->SetTitle("Day of Year");
  currentEnergyGraph.GetYaxis()->SetTitle("Energy [kWh]");

  TGraphAsymmErrors currentNormalizedEnergyGraph;
  graphName = "normalizedEnergyGraph_tree";
  currentNormalizedEnergyGraph.SetName(graphName.c_str());
  currentNormalizedEnergyGraph.SetTitle("");
  currentNormalizedEnergyGraph.GetXaxis()->SetTitle("Day of Year");
  currentNormalizedEnergyGraph.GetYaxis()->SetTitle("Fractional Energy");

  TGraphAsymmErrors currentEnergyDensityGraph;
  graphName = "energyDensityGraph_tree";
  currentEnergyDensityGraph.SetName(graphName.c_str());
  currentEnergyDensityGraph.SetTitle("");
  currentEnergyDensityGraph.GetXaxis()->SetTitle("Day of Year");
  currentEnergyDensityGraph.GetYaxis()->SetTitle("Energy density [kWhm^{-2}]");

  TGraphAsymmErrors currentMonthlyEnergyDensityGraph;
  graphName = "monthlyEnergyDensityGraph_tree";
  currentMonthlyEnergyDensityGraph.SetName(graphName.c_str());
  currentMonthlyEnergyDensityGraph.SetTitle("");
  currentMonthlyEnergyDensityGraph.GetXaxis()->SetTitle("Month");
  currentMonthlyEnergyDensityGraph.GetYaxis()->SetTitle(
      "Energy density [kWhm^{-2}]");

  TGraphAsymmErrors currentYearlyEnergyDensityGraph;
  graphName = "yearlyEnergyDensityGraph_tree";
  currentYearlyEnergyDensityGraph.SetName(graphName.c_str());
  currentYearlyEnergyDensityGraph.SetTitle("");
  currentYearlyEnergyDensityGraph.GetXaxis()->SetTitle("Year");
  currentYearlyEnergyDensityGraph.GetYaxis()->SetTitle(
      "Energy density [kWhm^{-2}]");

  // Get simulation values
  //     YearlyResult* currentStructure      = (YearlyResult*)scanIterator();
  double currentSensitiveArea =
      currentStructure->getTree()->getDoubleParameter("sensitiveArea");
  std::vector<time_t> dayTimes = currentStructure->getDayTimes();
  std::vector<double> energyDeposited = currentStructure->getEnergyDeposited();
  double totalYearEnergySum =
      currentStructure->getTree()->getDoubleParameter("totalIntegratedEnergyDeposit");
  std::cout << "Got info: totalYearEnergySum = " << totalYearEnergySum
            << std::endl;
  std::cout << "Got info: energy deposited size = " << energyDeposited.size()
            << std::endl;
  std::cout << "Got info: sensitive area = " << currentSensitiveArea
            << std::endl;

  // Check if the sensitive area is large enough
  //     if (currentSensitiveArea <= minimumSensitiveArea){
  //       delete currentStructure;
  //       continue;
  //     }

  // Check energy sum is positive (why would it not?!)
  //   if (totalYearEnergySum <= 0.0){
  //     delete currentStructure;
  //     continue;
  //   }

  // Fill graphs with days which have all simulated days
  std::cout << "Got info: day times size = " << dayTimes.size() << std::endl;
  for (unsigned int d = 0; d < dayTimes.size(); d++) {
    // Add the point to the graph
    time_t currentTime = dayTimes[d];
    long rootTime = offsetToRootTime(currentTime);

    int nextPointIndex = currentEnergyGraph.GetN();
    currentEnergyGraph.SetPoint(nextPointIndex, rootTime, energyDeposited[d]);
    currentNormalizedEnergyGraph.SetPoint(
        nextPointIndex, rootTime, energyDeposited[d] / totalYearEnergySum);
    currentEnergyDensityGraph.SetPoint(
        nextPointIndex, rootTime, energyDeposited[d] / currentSensitiveArea);
    std::cout << "Point set: " << rootTime << " energy: " << energyDeposited[d]
              << std::endl;
  }

  // Fill integrated monthly graphs
  fillGraphWithIntegratedMonth(currentMonthlyEnergyDensityGraph,
                               currentStructure);

  // Fill integrated yearly graphs
  fillGraphWithIntegratedYear(currentYearlyEnergyDensityGraph,
                              currentStructure);

  // Store for later writing.
  //   energyGraphs.push_back(currentEnergyGraph);
  //   normalizedEnergyGraphs.push_back(currentNormalizedEnergyGraph);
  //   energyDensityGraphs.push_back(currentEnergyDensityGraph);
  //   monthlyEnergyDensityGraphs.push_back(currentMonthlyEnergyDensityGraph);
  //   yearlyEnergyDensityGraphs.push_back(currentYearlyEnergyDensityGraph);

  //     delete currentStructure;

  // Store the average of all the trees
  //   TGraphAsymmErrors averageTreeEnergyGraph;
  //   TGraphAsymmErrors averageTreeNormalizedEnergyGraph;
  //   TGraphAsymmErrors averageTreeEnergyDensityGraph;
  //   TGraphAsymmErrors averageTreeMonthlyEnergyDensityGraph;
  //   TGraphAsymmErrors averageTreeYearlyEnergyDensityGraph;

  // Setup the average graph properties
  //   averageTreeEnergyGraph.SetName("averageTreeEnergyGraph");
  //   averageTreeEnergyGraph.SetTitle("");
  //   averageTreeEnergyGraph.GetXaxis()->SetTitle("Time");
  //   averageTreeEnergyGraph.GetYaxis()->SetTitle("Energy [kWh]");
  //   averageTreeEnergyGraph.GetXaxis()->SetTimeDisplay(1);
  //   averageTreeEnergyGraph.GetXaxis()->SetTimeFormat("%d/%m/%Y");

  //   averageTreeNormalizedEnergyGraph.SetName("averageTreeNormalizedEnergyGraph");
  //   averageTreeNormalizedEnergyGraph.SetTitle("");
  //   averageTreeNormalizedEnergyGraph.GetXaxis()->SetTitle("Time");
  //   averageTreeNormalizedEnergyGraph.GetYaxis()->SetTitle("Fractional
  //   Energy");
  //   averageTreeNormalizedEnergyGraph.GetXaxis()->SetTimeDisplay(1);
  //   averageTreeNormalizedEnergyGraph.GetXaxis()->SetTimeFormat("%d/%m/%Y");

  //   averageTreeEnergyDensityGraph.SetName("averageTreeEnergyDensityGraph");
  //   averageTreeEnergyDensityGraph.SetTitle("");
  //   averageTreeEnergyDensityGraph.GetXaxis()->SetTitle("Time");
  //   averageTreeEnergyDensityGraph.GetYaxis()->SetTitle("Energy density
  //   [kWhm^{-2}]");
  //   averageTreeEnergyDensityGraph.GetXaxis()->SetTimeDisplay(1);
  //   averageTreeEnergyDensityGraph.GetXaxis()->SetTimeFormat("%d/%m/%Y");

  //   averageTreeMonthlyEnergyDensityGraph.SetName("averageTreeMonthlyEnergyDensityGraph");
  //   averageTreeMonthlyEnergyDensityGraph.SetTitle("");
  //   averageTreeMonthlyEnergyDensityGraph.GetXaxis()->SetTitle("Time");
  //   averageTreeMonthlyEnergyDensityGraph.GetYaxis()->SetTitle("Energy density
  //   [kWhm^{-2}]");
  //   averageTreeMonthlyEnergyDensityGraph.GetXaxis()->SetTimeDisplay(1);
  //   averageTreeMonthlyEnergyDensityGraph.GetXaxis()->SetTimeFormat("%m/%Y");

  //   averageTreeYearlyEnergyDensityGraph.SetName("averageTreeYearlyEnergyDensityGraph");
  //   averageTreeYearlyEnergyDensityGraph.SetTitle("");
  //   averageTreeYearlyEnergyDensityGraph.GetXaxis()->SetTitle("Time");
  //   averageTreeYearlyEnergyDensityGraph.GetYaxis()->SetTitle("Energy density
  //   [kWhm^{-2}]");
  //   averageTreeYearlyEnergyDensityGraph.GetXaxis()->SetTimeDisplay(1);
  //   averageTreeYearlyEnergyDensityGraph.GetXaxis()->SetTimeFormat("%m/%Y");

  // Create the averages from previously filled graphs
  //   createAverageGraph(averageTreeEnergyGraph,           energyGraphs);
  //   createAverageGraph(averageTreeNormalizedEnergyGraph,
  //   normalizedEnergyGraphs);
  //   createAverageGraph(averageTreeEnergyDensityGraph, energyDensityGraphs);
  //   createAverageGraph(averageTreeMonthlyEnergyDensityGraph,
  //   monthlyEnergyDensityGraphs);
  //   createAverageGraph(averageTreeYearlyEnergyDensityGraph,
  //   yearlyEnergyDensityGraphs);

  // Prepare a root file to store the results
  TFile resultsFile(outputFileName.c_str(), "RECREATE");

  // Make a canvas combining all the plots in one summary graphic
  createSummaryCanvas(currentEnergyGraph, "energySummaryCanvas", "Time",
                      "Energy per day[kWh/day]");
  createSummaryCanvas(currentNormalizedEnergyGraph, "normalizedSummaryCanvas",
                      "Time", "Fractional Energy per day");
  createSummaryCanvas(currentEnergyDensityGraph, "energyDensityCanvas", "Time",
                      "Energy density per day [kWhm^{-2}/day]");

  // Save the current plots
  currentEnergyGraph.Write();
  currentNormalizedEnergyGraph.Write();
  currentEnergyDensityGraph.Write();
  currentMonthlyEnergyDensityGraph.Write();
  currentYearlyEnergyDensityGraph.Write();

  // Close the root file
  resultsFile.Close();
}

/*! \brief Time binned energy plotter main.
 *
 * Display simulation results over periods involving many days. Uses both actual
 *simulated
 * days and interpolation results. Binning is also applied for monthly periods.
 *
 * @param[in] argc Number of command line arguments.
 * @param[in] argv See yearlyEnergyPlotter --help (showHelp) for available
 *arguments.
 *
 */
int main(int argc, char** argv) {
  //   unsigned int treeNumber;
  std::string inputTreeFileName;
  std::string outputFileName;
  //   double minimumSensitiveArea;

  GetOpt::GetOpt_pp ops(argc, argv);

  // Check for help request
  if (ops >> GetOpt::OptionPresent('h', "help")) {
    showHelp();
    return 0;
  }

  //   ops >> GetOpt::Option("treeNumber", treeNumber, 10u);
  ops >> GetOpt::Option("inputTreeFile", inputTreeFileName, "");
  //   ops >> GetOpt::Option("minimumSensitiveArea", minimumSensitiveArea,
  //   0.001);
  ops >> GetOpt::Option("outputFileName", outputFileName,
                        "yearlyEnergyPlotter.results.root");

  if (inputTreeFileName == "") {
    std::cerr << "No tree file specified." << std::endl;
    showHelp();
    return 1;
  }

  // Report input parameters
  std::cout << "Just using trees from " << inputTreeFileName << std::endl;
  //   std::cout << "Plotting "    << treeNumber << " trees." << std::endl;
  std::cout << "Recording results in " << outputFileName << std::endl;

  // Also do not run if other arguments are present
  if (ops.options_remain()) {
    std::cerr << "Oops! Unexpected options." << std::endl;
    showHelp();
    return 1;
  }

  // Load PVTree environment
  pvtree::loadEnvironment();

  // Load from file
  TFile inputTreeFile(inputTreeFileName.c_str(), "READ");
  TList* scanResults = (TList*)inputTreeFile.FindObjectAny("testedStructures");
  TIter scanIterator(scanResults);

  // find the best result
  int counter{0};
  int id{0};
  double eff, energy, area;
  double lai;
  double sx, sy;
  double besteff{0.0};
  while (YearlyResult* currentStructure = (YearlyResult*)scanIterator()) {
    TreeConstructionInterface* clonedT = currentStructure->getTree();
    area = clonedT->getDoubleParameter("sensitiveArea");
    energy = clonedT->getDoubleParameter("totalIntegratedEnergyDeposit");
    sx = clonedT->getDoubleParameter("structureXSize");
    sy = clonedT->getDoubleParameter("structureYSize");
    lai = area / (sx * sy);

    eff = energy * lai;
    if (eff > besteff) {  // book best tree
      id = counter;
      besteff = eff;
      std::cout << "Tree ID: " << id << "; Best efficiency = " << besteff
                << std::endl;
    }
    counter++;
  }

  // Fill the graphs
  fillGraphs((YearlyResult*)scanResults->At(id), outputFileName);

  inputTreeFile.Close();

  return 0;
}
