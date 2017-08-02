/*! @file
 * \brief Plot the effect on the integrated irradiance
 *        of the various atmosphere properties. Will be mainly
 *        a test of SMARTS and SolPos settings.
 *
 */
#include "pvtree/full/solarSimulation/spectrumFactory.hpp"
#include "pvtree/location/locationDetails.hpp"
#include "pvtree/climate/climateFactory.hpp"
#include "pvtree/climate/climate.hpp"
#include "pvtree/full/solarSimulation/sun.hpp"
#include "pvtree/analysis/rootStyles.hpp"
#include "pvtree/utils/getopt_pp.h"
#include "pvtree/utils/resource.hpp"
#include "eccodes.h"

#include <iostream>
#include <cstring>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <regex>
#include <assert.h>

#include <CLHEP/Units/SystemOfUnits.h>

// save diagnostic state
#pragma GCC diagnostic push

// turn off the specific warning.
#pragma GCC diagnostic ignored "-Wshadow"

#include "TFile.h"
#include "TAxis.h"
#include "TCanvas.h"
#include "TGraphAsymmErrors.h"
#include "TLegend.h"
#include "TColor.h"
#include "TH1D.h"
#include "TGaxis.h"

// turn the warnings back on
#pragma GCC diagnostic pop

using CLHEP::gram;
using CLHEP::cm2;
using CLHEP::kilogram;
using CLHEP::m2;

void showHelp() {
  std::cout << "smartsClimateAffect help" << std::endl;
  std::cout << "\t --samplesPerDay <Long>; Default 15" << std::endl;
  std::cout << "\t --startDate <Date String> :\t default 1/1/2010" << std::endl;
  std::cout << "\t --endDate <Date String> :\t default 1/1/2015" << std::endl;
  std::cout << "\t --spectrumName <String>; Default 'Direct_normal_irradiance'"
            << std::endl;
  std::cout << "\t --outputFileName <String>; Default 'climateAffect'"
            << std::endl;
  std::cout
      << "\n\t This is quite slow, you might want to limit the date range!"
      << std::endl;
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

/*! \brief Create a list of monthly bin low edges in standard unix time
 *
 * @param[in] startTime Time containing the starting month
 * @param[in] endTime Time containing the ending month
 */
std::vector<time_t> createMonthlyBinEdges(time_t startTime, time_t endTime) {
  // Need structure to hold the calendar times
  struct tm* calendarTime = gmtime(&endTime);

  // Need to extract end numbers because there can be only one tm structure
  int endYear = calendarTime->tm_year;
  int endMonth = calendarTime->tm_mon;

  calendarTime = gmtime(&startTime);

  std::vector<time_t> monthBinLowEdges;

  while (calendarTime->tm_year <= endYear) {
    if (calendarTime->tm_year == endYear && calendarTime->tm_mon > endMonth) {
      // Done
      break;
    }

    // Record a new low edge (offset to root time)
    monthBinLowEdges.push_back(mktime(calendarTime));

    // Increment the calendar time
    if (calendarTime->tm_mon + 1 > 11) {
      calendarTime->tm_mon = 0;
      calendarTime->tm_year += 1;
    } else {
      calendarTime->tm_mon += 1;
    }
  }

  return monthBinLowEdges;
}

/*! \brief Create a graph of the summed energy per unit area for each of
 *         the specified time periods. Simply takes the summed irradiance from
 *the
 *         SMARTS spectrum, which is sampled a number of times per day.
 *
 * @param[in] graphName Name of graph to produce.
 * @param[in] spectrumName The name of SMARTS output histogram to be summed.
 * @param[in] samplesPerDay The Number of spectral evaluations carried out per
 *day.
 * @param[in] sun The instance of Sun to be used to access the spectra.
 * @param[in] lowBinEdges The binning to be used for the time axis.
 *
 * \returns A copy of the filled graph.
 */
TGraphAsymmErrors createSummedSpectralGraph(std::string graphName,
                                            std::string spectrumName,
                                            long samplesPerDay, Sun& sun,
                                            std::vector<time_t> lowBinEdges) {
  // Output step currently on
  std::cout << "Creating " << graphName << " graph with " << samplesPerDay
            << " samples per day." << std::endl;

  TGraphAsymmErrors graph;
  graph.SetName(graphName.c_str());

  // Integrate energy in monthly period
  for (unsigned int m = 0; m < lowBinEdges.size() - 1; m++) {
    double energySum = 0.0;
    time_t currentDay = lowBinEdges[m];
    time_t startOfNextPeriod =
        lowBinEdges[m + 1] -
        600;  // (subtract ten minutes to make comparison simpler)

    // Iterate over days
    while (currentDay < startOfNextPeriod) {
      // Use sun to get simulation time range
      sun.setDate(currentDay);
      int sunriseTime = sun.getSunriseTime() * 60;  // s
      int sunsetTime = sun.getSunsetTime() * 60;  // s
      double simulationStepTime =
          static_cast<double>(sunsetTime - sunriseTime) / samplesPerDay;

      // Integrate over day
      for (unsigned int timeIndex = 0; timeIndex < samplesPerDay; timeIndex++) {
        // Set the time to the mid-point of the day-time segment
        sun.setTime((int)(sunriseTime + timeIndex * simulationStepTime +
                          simulationStepTime / 2.0));

        // Get the direct normal irradiance
        double irradiance =
            sun.getSpectrum()->getHistogram(spectrumName)->Integral("width");

        energySum += irradiance * simulationStepTime;
      }

      // Change the day
      currentDay += 24.0 * 60.0 * 60.0;
    }

    // Convert from Wseconds to kWhours
    energySum *= 1.0 / (60.0 * 60.0 * 1000.0);

    int nextPointIndex = graph.GetN();
    time_t midPoint = (lowBinEdges[m + 1] + lowBinEdges[m]) / 2.0;

    // Add a new point to the graph
    graph.SetPoint(nextPointIndex, offsetToRootTime(midPoint), energySum);
  }

  return graph;
}

/*! \brief Create a canvas from a set of plots which are all overlayed on the
 *         same pad. It will save it both to the current root file and export to
 *         an eps file. The x-axis will be set to time format.
 *
 * @param[in] canvasName Unique name of canvas.
 * @param[in] plots A list of plots that should be shown on the canvas.
 * @param[in] options A list of drawing options describing how each plot should
 *                    be displayed.
 * @param[in] colours A list of colour options for each plot.
 * @param[in] xAxisTitle The title on the x axis.
 * @param[in] yAxisTitle The title on the y axis.
 *
 */
template <typename T>
void createCanvas(std::string canvasName, std::vector<T> plots,
                  std::vector<std::string> options, std::vector<int> colours,
                  std::string xAxisTitle, std::string yAxisTitle) {
  int canvasWidth = 3500;
  int canvasHeight = 300;

  TCanvas canvas(canvasName.c_str(), "", canvasWidth, canvasHeight);
  plots[0].Draw(options[0].c_str());
  plots[0].SetTitle("");
  plots[0].GetXaxis()->SetTitle(xAxisTitle.c_str());
  plots[0].GetYaxis()->SetTitle(yAxisTitle.c_str());
  plots[0].GetXaxis()->SetTimeDisplay(1);
  plots[0].GetXaxis()->SetTimeFormat("%d/%m/%Y");
  plots[0].SetLineColor(colours[0]);

  for (unsigned int p = 1; p < plots.size(); p++) {
    plots[p].Draw(options[p].c_str());
    plots[p].SetLineColor(colours[p]);
  }

  canvas.Update();
  canvas.Write();

  // Dump out an eps as well
  canvas.SaveAs((canvasName + ".eps").c_str());
}

/*! \brief Create a ratio plot from TGraphAsymmErrors
 *
 * @param[in] graph1 The denominator graph.
 * @param[in] graph2 The numerator graph.
 * \returns The created ratio plot.
 */
TGraphAsymmErrors createRatioPlot(TGraphAsymmErrors graph1,
                                  TGraphAsymmErrors graph2) {
  TGraphAsymmErrors ratioPlot;

  if (graph1.GetN() != graph2.GetN()) {
    std::cerr
        << "Cannot create ratio plot as graphs have different numbers of points"
        << std::endl;
    return ratioPlot;
  }

  for (int p = 0; p < graph1.GetN(); p++) {
    double xValue = graph1.GetX()[p];
    double yValue = 0.0;

    if (graph1.GetY()[p] != 0.0) {
      yValue = graph2.GetY()[p] / graph1.GetY()[p];
    }

    ratioPlot.SetPoint(p, xValue, yValue);
  }

  return ratioPlot;
}

/*! \brief Calculate the y axis range of a graph based upon the points
 *contained.
 *
 * @param[in] graph The reference of a graph to be checked.
 */
void setPlotYAxisRange(TGraphAsymmErrors& graph) {
  double minY = 0.0, maxY = 0.0;

  for (int p = 0; p < graph.GetN(); p++) {
    double yValue = graph.GetY()[p];

    if (p == 0) {
      minY = yValue;
      maxY = yValue;
      continue;
    }

    if (minY > yValue) {
      minY = yValue;
    }

    if (maxY < yValue) {
      maxY = yValue;
    }
  }

  graph.SetMaximum(maxY);
  graph.SetMinimum(minY);
}

/*! \brief Create a canvas from a set of plots which are all overlayed on the
 *         same pad. It will save it both to the current root file and export to
 *         an eps file. The x-axis will be set to time format. In addition a
 *ratio
 *         plot will be shown in a connected pad.
 *
 * @param[in] canvasName Unique name of canvas.
 * @param[in] plots A list of plots that should be shown on the canvas.
 * @param[in] options A list of drawing options describing how each plot should
 *                    be displayed.
 * @param[in] colours A list of colour options for each plot.
 * @param[in] markerStyles A list of marker style options for each plot.
 * @param[in] xAxisTitle The title on the x axis.
 * @param[in] yAxisTitle The title on the y axis.
 * @param[in] legendLabels List of legend label for each of the overlayed plots
 * @param[in] legendOptions List of legend draw options for each of the
 *overlayed
 *                          plots
 *
 */
template <typename T>
void createRatioCanvas(std::string canvasName, std::vector<T> plots,
                       std::vector<std::string> options,
                       std::vector<int> colours, std::vector<int> markerStyles,
                       std::string xAxisTitle, std::string yAxisTitle,
                       std::vector<std::string> legendLabels,
                       std::vector<std::string> legendOptions) {
  int canvasWidth = 3500;
  int canvasHeight = 1200;

  // Allow variation in plot heights.
  double topYFraction = 0.6;
  double bottomYFraction = 1.0 - topYFraction;

  if (plots.size() != options.size() && plots.size() != colours.size() &&
      plots.size() != markerStyles.size() &&
      plots.size() != legendLabels.size() &&
      plots.size() != legendOptions.size()) {
    std::cerr << "Inconsistent input arrays, and so cannot create "
              << canvasName << std::endl;
    return;
  }

  // Create a canvas with two pads
  TCanvas canvas(canvasName.c_str(), "", canvasWidth, canvasHeight);

  TPad topPad("TopPad", "", 0.0, 1.0 - topYFraction, 1.0, 1.0);
  canvas.cd();
  topPad.AppendPad();
  TPad bottomPad("BottomPad", "", 0.0, 0.0, 1.0, bottomYFraction);
  canvas.cd();
  bottomPad.AppendPad();

  // Scale label/title sizes
  double topTextScaleValue = bottomYFraction / topYFraction;

  std::vector<TGaxis*> temporaryAxis;

  // Draw axis for only a fraction of plot to avoid problems at the edge
  // of the stacked plot. Be careful not to go too big otherwise infinite loop!
  double axisPaddingFraction = 0.07;
  assert(axisPaddingFraction < 0.5);

  // Fill the top pad
  topPad.cd();
  topPad.SetFrameBorderMode(0);
  topPad.SetBorderMode(0);
  topPad.SetBorderSize(0);
  topPad.SetTopMargin(0.05);
  topPad.SetBottomMargin(0.0);

  // Ensure that the complete range of curves is shown
  double minimalYValue = 0.0, maximalYValue = 0.0;

  for (unsigned int p = 0; p < plots.size(); p++) {
    setPlotYAxisRange(plots[p]);

    if (p == 0) {
      minimalYValue = plots[p].GetMinimum();
      maximalYValue = plots[p].GetMaximum();
      continue;
    }

    if (plots[p].GetMinimum() < minimalYValue) {
      minimalYValue = plots[p].GetMinimum();
    }

    if (plots[p].GetMaximum() > maximalYValue) {
      maximalYValue = plots[p].GetMaximum();
    }
  }

  // Add some padding to the maximum and minimum values
  double absolutePaddingSize =
      std::fabs(maximalYValue - minimalYValue) * axisPaddingFraction;
  maximalYValue += absolutePaddingSize;
  minimalYValue -= absolutePaddingSize;

  for (unsigned int p = 0; p < plots.size(); p++) {
    plots[p].Draw(options[p].c_str());
    plots[p].SetTitle("");
    plots[p].SetLineColor(colours[p]);
    plots[p].SetFillColor(colours[p]);
    plots[p].SetMarkerColor(colours[p]);
    plots[p].SetMarkerStyle(markerStyles[p]);
    plots[p].GetYaxis()->SetRangeUser(minimalYValue, maximalYValue);
    plots[p].SetLineWidth(2);
    plots[p].SetMarkerSize(4);

    // Draw another y-axis on the right side as a test
    // Option 'S' means ticklength = fTickSize*axis_length
    if (p == 0) {
      TGaxis* yAxis = new TGaxis(
          plots[p].GetXaxis()->GetXmin(), minimalYValue + absolutePaddingSize,
          plots[p].GetXaxis()->GetXmin(), maximalYValue - absolutePaddingSize,
          minimalYValue + absolutePaddingSize,
          maximalYValue - absolutePaddingSize, 506, "S");
      yAxis->ImportAxisAttributes(plots[p].GetYaxis());
      yAxis->SetLineColor(kBlack);
      yAxis->SetTextColor(kBlack);
      yAxis->SetTitle(yAxisTitle.c_str());
      yAxis->CenterTitle(true);

      yAxis->SetLabelSize(topTextScaleValue * yAxis->GetLabelSize());
      yAxis->SetTitleSize(topTextScaleValue * yAxis->GetTitleSize());
      yAxis->SetTitleOffset(yAxis->GetTitleOffset() / topTextScaleValue);

      yAxis->Draw("");

      temporaryAxis.push_back(yAxis);

      // Hide original axis
      plots[p].GetXaxis()->SetTitleSize(0);
      plots[p].GetXaxis()->SetNdivisions(0);

      plots[p].GetYaxis()->SetTitleSize(0);
      plots[p].GetYaxis()->SetNdivisions(0);
    }
  }

  // Create a legend
  double legendLabelSize = (0.91 - 0.53) / 4.0;
  TLegend legend(0.081, 0.91 - legendLabelSize * plots.size(), 0.21, 0.91);

  for (unsigned int p = 0; p < plots.size(); p++) {
    legend.AddEntry(&(plots[p]), legendLabels[p].c_str(),
                    legendOptions[p].c_str());
  }

  legend.Draw();

  // Fill the ratio pad
  // Assume making ratio with respect to the first plot
  bottomPad.cd();
  bottomPad.SetFrameBorderMode(0);
  bottomPad.SetBorderMode(0);
  bottomPad.SetBorderSize(0);
  bottomPad.SetTopMargin(0.0);
  bottomPad.SetBottomMargin(0.28);

  std::vector<T> ratioPlots;

  for (unsigned int p = 0; p < plots.size(); p++) {
    ratioPlots.push_back(createRatioPlot(plots[0], plots[p]));
  }

  // Ensure that the complete range of curves is shown
  minimalYValue = 0.9;
  maximalYValue = 1.1;

  for (unsigned int p = 0; p < ratioPlots.size(); p++) {
    setPlotYAxisRange(ratioPlots[p]);

    if (ratioPlots[p].GetMinimum() < minimalYValue) {
      minimalYValue = ratioPlots[p].GetMinimum();
    }

    if (ratioPlots[p].GetMaximum() > maximalYValue) {
      maximalYValue = ratioPlots[p].GetMaximum();
    }
  }

  // Add some padding to the maximum and minimum values
  absolutePaddingSize =
      std::fabs(maximalYValue - minimalYValue) * axisPaddingFraction;
  maximalYValue += absolutePaddingSize;
  minimalYValue -= absolutePaddingSize;

  for (unsigned int p = 0; p < ratioPlots.size(); p++) {
    ratioPlots[p].Draw(options[p].c_str());
    ratioPlots[p].SetTitle("");
    ratioPlots[p].SetLineColor(colours[p]);
    ratioPlots[p].SetFillColor(colours[p]);
    ratioPlots[p].SetMarkerColor(colours[p]);
    ratioPlots[p].SetMarkerStyle(markerStyles[p]);
    ratioPlots[p].GetYaxis()->SetRangeUser(minimalYValue, maximalYValue);
    ratioPlots[p].SetLineWidth(2);
    ratioPlots[p].SetMarkerSize(4);

    // Draw another y-axis on the right side as a test
    // Option 'S' means ticklength = fTickSize*axis_length
    if (p == 0) {
      TGaxis* yAxis = new TGaxis(ratioPlots[p].GetXaxis()->GetXmin(),
                                 minimalYValue + absolutePaddingSize,
                                 ratioPlots[p].GetXaxis()->GetXmin(),
                                 maximalYValue - absolutePaddingSize,
                                 minimalYValue + absolutePaddingSize,
                                 maximalYValue - absolutePaddingSize, 506, "S");
      yAxis->ImportAxisAttributes(ratioPlots[p].GetYaxis());
      yAxis->SetLineColor(kBlack);
      yAxis->SetTextColor(kBlack);
      yAxis->SetTitle("Ratio");
      yAxis->CenterTitle(true);
      yAxis->Draw("");

      temporaryAxis.push_back(yAxis);

      // Repeat for x-axis
      absolutePaddingSize = std::fabs(ratioPlots[p].GetXaxis()->GetXmax() -
                                      ratioPlots[p].GetXaxis()->GetXmin()) *
                            axisPaddingFraction;

      // Option 't' for time and 'S' for ticklength as above.
      TGaxis* xAxis = new TGaxis(
          ratioPlots[p].GetXaxis()->GetXmin() + absolutePaddingSize,
          minimalYValue,
          ratioPlots[p].GetXaxis()->GetXmax() - absolutePaddingSize,
          minimalYValue,
          ratioPlots[p].GetXaxis()->GetXmin() + absolutePaddingSize,
          ratioPlots[p].GetXaxis()->GetXmax() - absolutePaddingSize, 506, "tS");
      xAxis->ImportAxisAttributes(ratioPlots[p].GetXaxis());
      xAxis->SetLineColor(kBlack);
      xAxis->SetTextColor(kBlack);
      xAxis->SetTitle(xAxisTitle.c_str());
      xAxis->SetTitleOffset(0.85);
      xAxis->CenterTitle(false);

      TDatime da(1995, 01, 1, 00, 00, 00);
      xAxis->SetTimeOffset(da.Convert());
      xAxis->SetTimeFormat("%d/%m/%Y");

      xAxis->Draw("");

      temporaryAxis.push_back(xAxis);

      // Hide original axis
      ratioPlots[p].GetXaxis()->SetTitleSize(0);
      ratioPlots[p].GetXaxis()->SetNdivisions(0);

      ratioPlots[p].GetYaxis()->SetTitleSize(0);
      ratioPlots[p].GetYaxis()->SetNdivisions(0);
    }
  }

  canvas.Update();
  canvas.Write();

  // Dump out an eps as well
  canvas.SaveAs((canvasName + ".eps").c_str());

  // Clean up all the axis created
  for (auto& axis : temporaryAxis) {
    delete axis;
  }
  temporaryAxis.clear();
}

/*! \brief Program main for smartsClimateAffect
 *
 * See showHelp for details of commandline options
 */
int main(int argc, char** argv) {
  long samplesPerDay;
  std::string spectrumName;
  std::string outputFileName;
  std::string startDate;
  std::string endDate;

  GetOpt::GetOpt_pp ops(argc, argv);

  // Check for help request
  if (ops >> GetOpt::OptionPresent('h', "help")) {
    showHelp();
    return 0;
  }

  ops >> GetOpt::Option("samplesPerDay", samplesPerDay, 15l);
  ops >> GetOpt::Option("spectrumName", spectrumName,
                        std::string("Direct_normal_irradiance"));
  ops >> GetOpt::Option("outputFileName", outputFileName,
                        std::string("climateAffect"));
  ops >> GetOpt::Option("startDate", startDate, "1/1/2010");
  ops >> GetOpt::Option("endDate", endDate, "1/1/2015");

  // Also do not run if other arguments are present
  if (ops.options_remain()) {
    std::cerr << "Oops! Unexpected options." << std::endl;
    showHelp();
    return -1;
  }

  pvtree::loadEnvironment();

  // Set the plotting style to 'flat' for the Solar Energy journal
  Style_SolarEnergyFlat();

  // Get the list of months
  std::vector<time_t> monthLowBinEdges =
      createMonthlyBinEdges(interpretDate(startDate), interpretDate(endDate));

  // Get the device location details
  LocationDetails deviceLocation("location.cfg");

  // Obtain the simulation sun
  Sun sun(deviceLocation);

  // Turn off some of the suns climate options
  sun.setClimateOption(Sun::CLOUDCOVER, false);

  // Prepare the climate factory with the default configuration
  ClimateFactory::instance()->setConfigurationFile("default.cfg");
  ClimateFactory::instance()->setDeviceLocation(deviceLocation);

  // Get the spectrum factory
  SpectrumFactory* factory = SpectrumFactory::instance();

  // Set the altitude for specific device location and
  // add extra variables to calculate
  factory->setAltitude(deviceLocation.getAltitude());
  factory->appendOutputVariable(3);  // diffuse horizontal irradiance
  factory->appendOutputVariable(5);  // direct horizontal irradiance

  // Default clear sky setup ( compare everything to this! )
  TGraphAsymmErrors clearSkyGraph =
      createSummedSpectralGraph("clearSkyMonthlySpectralSum", spectrumName,
                                samplesPerDay, sun, monthLowBinEdges);

  // All climate options turned on (need to reset spectrum factory).
  sun.setClimateOption(Sun::CLOUDCOVER, true);
  factory->setDefaults();
  factory->setAltitude(deviceLocation.getAltitude());
  TGraphAsymmErrors allClimateGraph =
      createSummedSpectralGraph("allClimateMonthlySpectralSum", spectrumName,
                                samplesPerDay, sun, monthLowBinEdges);

  // Turn off temperature + cloud cover
  sun.setClimateOption(Sun::CLOUDCOVER, false);
  sun.setClimateOption(Sun::TEMPERATURE, false);
  factory->setDefaults();
  factory->setAltitude(deviceLocation.getAltitude());
  TGraphAsymmErrors clearSkyNoTemperatureGraph = createSummedSpectralGraph(
      "clearSkyNoTemperatureMonthlySpectralSum", spectrumName, samplesPerDay,
      sun, monthLowBinEdges);

  // Turn off pressure + cloud cover
  sun.setClimateOption(Sun::PRESSURE, false);
  sun.setClimateOption(Sun::TEMPERATURE, true);
  factory->setDefaults();
  factory->setAltitude(deviceLocation.getAltitude());
  TGraphAsymmErrors clearSkyNoPressureGraph = createSummedSpectralGraph(
      "clearSkyNoPressureMonthlySpectralSum", spectrumName, samplesPerDay, sun,
      monthLowBinEdges);

  // Turn off column water + cloud cover
  sun.setClimateOption(Sun::COLUMNWATER, false);
  sun.setClimateOption(Sun::PRESSURE, true);
  factory->setDefaults();
  factory->setAltitude(deviceLocation.getAltitude());
  TGraphAsymmErrors clearSkyNoColumnWaterGraph = createSummedSpectralGraph(
      "clearSkyNoColumnWaterMonthlySpectralSum", spectrumName, samplesPerDay,
      sun, monthLowBinEdges);

  // Turn off column ozone + cloud cover
  sun.setClimateOption(Sun::COLUMNOZONE, false);
  sun.setClimateOption(Sun::COLUMNWATER, true);
  factory->setDefaults();
  factory->setAltitude(deviceLocation.getAltitude());
  TGraphAsymmErrors clearSkyNoColumnOzoneGraph = createSummedSpectralGraph(
      "clearSkyNoColumnOzoneMonthlySpectralSum", spectrumName, samplesPerDay,
      sun, monthLowBinEdges);

  // Turn on heavy pollution on default clear sky
  sun.setClimateOption(Sun::COLUMNOZONE, true);
  factory->setDefaults();
  factory->setAltitude(deviceLocation.getAltitude());
  factory->setGasLoad(SpectrumFactory::SEVERE_POLLUTION);
  TGraphAsymmErrors clearSkyHeavyPollutionGraph = createSummedSpectralGraph(
      "clearSkyHeavyPollutionMonthlySpectralSum", spectrumName, samplesPerDay,
      sun, monthLowBinEdges);

  // Save the graphs to disk and export to eps files
  TFile resultsFile((outputFileName + ".root").c_str(), "RECREATE");

  // It takes approximately 20 mins to create all of these for a single year and
  // 5 samples per day.
  // SMARTS is not fast :D. This is why I need to run this as a separate step
  // during optimization.
  clearSkyGraph.Write();
  allClimateGraph.Write();
  clearSkyNoTemperatureGraph.Write();
  clearSkyNoPressureGraph.Write();
  clearSkyNoColumnWaterGraph.Write();
  clearSkyNoColumnOzoneGraph.Write();
  clearSkyHeavyPollutionGraph.Write();

  // Create a canvas combining these results
  createCanvas<TGraphAsymmErrors>(
      outputFileName + "_allSmartsInputVariationsCanvas",
      {clearSkyGraph, clearSkyNoTemperatureGraph, clearSkyNoPressureGraph,
       clearSkyNoColumnWaterGraph, clearSkyNoColumnOzoneGraph,
       clearSkyHeavyPollutionGraph, allClimateGraph},
      {"AL", "SAMEL", "SAMEL", "SAMEL", "SAMEL", "SAMEL", "SAMEL"},
      {kBlack, kBlue - 6, kGreen - 5, kRed - 5, kOrange + 2, kMagenta - 5,
       kRed - 8},
      "Date", "E [kWhm^{-2}month^{-1}]");

  createCanvas<TGraphAsymmErrors>(
      outputFileName + "_clearSkyInputVariationsCanvas",
      {clearSkyGraph, clearSkyNoTemperatureGraph, clearSkyNoPressureGraph,
       clearSkyNoColumnWaterGraph, clearSkyNoColumnOzoneGraph,
       clearSkyHeavyPollutionGraph},
      {"AL", "SAMEL", "SAMEL", "SAMEL", "SAMEL", "SAMEL"},
      {kBlack, kBlue - 6, kGreen - 5, kRed - 5, kOrange + 2, kMagenta - 5},
      "Date", "E [kWhm^{-2}month^{-1}]");

  // Also limit to just the 'important variables'
  createCanvas<TGraphAsymmErrors>(
      outputFileName + "_clearSkyImportantInputVariationsCanvas",
      {clearSkyGraph, clearSkyNoColumnWaterGraph, clearSkyNoColumnOzoneGraph,
       clearSkyHeavyPollutionGraph},
      {"AL", "SAMEL", "SAMEL", "SAMEL"},
      {kBlack, kBlue - 6, kGreen - 5, kRed - 5}, "Date",
      "E [kWhm^{-2}month^{-1}]");

  // Repeat above for ratio plots
  createRatioCanvas<TGraphAsymmErrors>(
      outputFileName + "_allSmartsInputVariationsRatioCanvas",
      {clearSkyGraph, clearSkyNoTemperatureGraph, clearSkyNoPressureGraph,
       clearSkyNoColumnWaterGraph, clearSkyNoColumnOzoneGraph,
       clearSkyHeavyPollutionGraph, allClimateGraph},
      {"APL", "SAMEPL", "SAMEPL", "SAMEPL", "SAMEPL", "SAMEPL", "SAMEPL"},
      {kBlack, kBlue - 6, kGreen - 5, kRed - 5, kOrange + 2, kMagenta - 5,
       kRed - 8},
      {20, 21, 22, 23, 33, 34, 29}, "Date", "E [kWhm^{-2}month^{-1}]",
      {"nominal", "reference temperature", "reference pressure",
       "reference column water", "reference column ozone", "heavy pollution",
       "cloudy"},
      {"LP", "LP", "LP", "LP", "LP", "LP", "LP"});

  createRatioCanvas<TGraphAsymmErrors>(
      outputFileName + "_clearSkyInputVariationsRatioCanvas",
      {clearSkyGraph, clearSkyNoTemperatureGraph, clearSkyNoPressureGraph,
       clearSkyNoColumnWaterGraph, clearSkyNoColumnOzoneGraph,
       clearSkyHeavyPollutionGraph},
      {"APL", "SAMEPL", "SAMEPL", "SAMEPL", "SAMEPL", "SAMEPL"},
      {kBlack, kBlue - 6, kGreen - 5, kRed - 5, kOrange + 2, kMagenta - 5},
      {20, 21, 22, 23, 33, 34}, "Date", "E [kWhm^{-2}month^{-1}]",
      {"nominal", "reference temperature", "reference pressure",
       "reference column water", "reference column ozone", "heavy pollution"},
      {"LP", "LP", "LP", "LP", "LP", "LP"});

  createRatioCanvas<TGraphAsymmErrors>(
      outputFileName + "_clearSkyImportantInputVariationsRatioCanvas",
      {clearSkyGraph, clearSkyNoColumnWaterGraph, clearSkyNoColumnOzoneGraph,
       clearSkyHeavyPollutionGraph},
      {"APL", "SAMEPL", "SAMEPL", "SAMEPL"},
      {kBlack, kBlue - 6, kGreen - 5, kRed - 5}, {20, 23, 33, 34}, "Date",
      "E [kWhm^{-2}month^{-1}]", {"nominal", "reference column water",
                                  "reference column ozone", "heavy pollution"},
      {"LP", "LP", "LP", "LP"});

  resultsFile.Close();

  return 0;
}
