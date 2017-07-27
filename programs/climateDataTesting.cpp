/*! @file climateDataTesting.cpp
 * \brief Testing of access to climate data.
 *
 * Initially just looking at examples in the documentation
 * https://software.ecmwf.int/wiki/display/ECC/GRIB+examples
 *
 */

#include <iostream>
#include <string>
#include <memory>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "pvtree/climate/climateFactory.hpp"
#include "pvtree/climate/climate.hpp"
#include "pvtree/full/solarSimulation/sun.hpp"
#include "pvtree/location/locationDetails.hpp"
#include "pvtree/analysis/rootStyles.hpp"
#include "pvtree/utils/getopt_pp.h"
#include "eccodes.h"

#include <CLHEP/Units/PhysicalConstants.h>

// save diagnostic state
#pragma GCC diagnostic push 

// turn off the specific warning.
#pragma GCC diagnostic ignored "-Wshadow"

#include "TFile.h"
#include "TGraphAsymmErrors.h"
#include "TAxis.h"
#include "TCanvas.h"
#include "TH2D.h"
#include "TStyle.h"
#include "TGaxis.h"

// turn the warnings back on
#pragma GCC diagnostic pop

void showHelp() {
  std::cout << "climateDataTesting help"     << std::endl;
  std::cout << "\t --daytimeOnly true|false" << std::endl;
}

double offsetToRootTime(time_t currentTime) {
  
  // ROOT defines time as starting from 01/01/1995, so I need to move the time to there
  struct tm calendarTime;
  calendarTime.tm_sec=0;
  calendarTime.tm_min=0; 
  calendarTime.tm_hour=1; 
  calendarTime.tm_mday=1; 
  calendarTime.tm_mon=0; 
  calendarTime.tm_year=95;
  calendarTime.tm_isdst=1;

  time_t rootStartTime = mktime(&calendarTime);

  // Convert to ROOT epoch time
  return difftime(currentTime, rootStartTime);
}

time_t getStartTime() {
  struct tm calendarTime;
  calendarTime.tm_sec =0;
  calendarTime.tm_min =0; 
  calendarTime.tm_hour=0; 
  calendarTime.tm_mday=1; 
  calendarTime.tm_mon =0; 
  calendarTime.tm_year=110;
  calendarTime.tm_isdst=1;
  return mktime(&calendarTime);
}

time_t getEndTime() {
  struct tm calendarTime;
  calendarTime.tm_sec =0;
  calendarTime.tm_min =0; 
  calendarTime.tm_hour=0; 
  calendarTime.tm_mday=1; 
  calendarTime.tm_mon =1; 
  calendarTime.tm_year=115;
  calendarTime.tm_isdst=1;
  return mktime(&calendarTime);
}

TGraphAsymmErrors createInterpolationGraph(std::string graphName, time_t startTime, time_t endTime, long timeStepSize, std::string valueName, std::function<double(double)> valueModifictionFunction = [](double v) { return v; } ) {

  TGraphAsymmErrors graph;
  graph.SetName(graphName.c_str());

  // How many steps should there be
  int stepNumber = (endTime - startTime)/timeStepSize;
  for (int stepCount=0; stepCount<stepNumber; stepCount++) {

    int nextPointIndex = graph.GetN();
    time_t currentTime = startTime + stepCount*timeStepSize;
    double currentValue = ClimateFactory::instance()->getClimate()->getInterpolatedValue(valueName, currentTime);

    // Value may need some transformation
    currentValue = valueModifictionFunction(currentValue);

    long rootTime = offsetToRootTime(currentTime);
    graph.SetPoint(nextPointIndex, rootTime, currentValue);

  }

  return graph;
}

std::vector<double> createMonthlyBinEdges(time_t startTime, time_t endTime) {
  
  // Need structure to hold the calendar times
  struct tm* calendarTime = gmtime( &endTime );

  // Need to extract end numbers because there can be only one tm structure
  int endYear = calendarTime->tm_year;
  int endMonth = calendarTime->tm_mon; 

  calendarTime = gmtime( &startTime );

  std::vector<double> monthBinLowEdges;

  while( calendarTime->tm_year <= endYear ) {
    
    if ( calendarTime->tm_year == endYear &&
	 calendarTime->tm_mon > endMonth) {
      // Done
      break;
    }

    // Record a new low edge (offset to root time)
    monthBinLowEdges.push_back( offsetToRootTime( mktime(calendarTime) ));

    // Increment the calendar time
    if ( calendarTime->tm_mon + 1 > 11 ){
      calendarTime->tm_mon   = 0;
      calendarTime->tm_year += 1;
    } else {
      calendarTime->tm_mon  += 1;
    }
  }

  return monthBinLowEdges;
}

/** \brief Create a candle plot for a climate variable using interpolation.
 *
 * The valueModificationFunction parameter is a transformation function applied to climate 
 * variable, which by default is a function which leaves the climate variable unchanged. Can't
 * use normal doxygen documentation because it doesn't recognise std::function! (old doxygen?)
 *
 * @param[in] plotName The name and title of the histogram.
 * @param[in] startTime The starting time of the x-axis.
 * @param[in] endTime The ending time of the x-axis.
 * @param[in] sampleStepSize The time step between sampling using interpolation.
 * @param[in] valueName Name of the climate variable being interpolated.
 * @param[in] daytimeOnly Check that all climate variables are sampled during the daytime if true.
 * @param[in] sun The object describing the suns behaviour at the test location.
 */
TH2D createCandlePlot(std::string plotName, 
		      time_t startTime, 
		      time_t endTime, 
		      long sampleStepSize, 
		      std::string valueName, 
		      bool daytimeOnly, 
		      Sun& sun, 
		      std::function<double(double)> valueModificationFunction = [](double v) { return v; } ){

  long sampleStepNumber = (endTime - startTime)/sampleStepSize;

  // Get the y-axis range
  double yAxisMin = 0.0;
  double yAxisMax = 0.0;
  bool setInitialValues = false;
  for (long t=0; t<sampleStepNumber; t++) {

    time_t currentTime = startTime + t*sampleStepSize;

    if ( daytimeOnly && !sun.isTimeDuringDay(currentTime) ) {
      continue;
    }

    double currentValue = ClimateFactory::instance()->getClimate()->getInterpolatedValue(valueName, currentTime);

    // Value may need some transformation
    currentValue = valueModificationFunction(currentValue);

    if (!setInitialValues){
      yAxisMin = currentValue;
      yAxisMax = currentValue;
      setInitialValues = true;
    } else {

      if (currentValue > yAxisMax) {
	yAxisMax = currentValue;
      }
      if (currentValue < yAxisMin) {
	yAxisMin = currentValue;
      }
    }

  }

  // Extend the y-axis a little bit so candles are fully visible
  double yAxisExtensionFraction = 0.1;
  double yAxisExtension = std::fabs(yAxisMax - yAxisMin) * yAxisExtensionFraction;
  yAxisMin -= yAxisExtension;
  yAxisMax += yAxisExtension;

  // Replace x axis with variable bin sizes based upon actual months start and stop
  //TH2D(const char* name, const char* title, Int_t nbinsx, const Double_t* xbins, Int_t nbinsy, Double_t ylow, Double_t yup)
  std::vector<double> monthlyBinLowEdgeVector = createMonthlyBinEdges(startTime, endTime);

  double* monthlyBinLowEdgeArray = new double[monthlyBinLowEdgeVector.size()];
  std::copy(monthlyBinLowEdgeVector.begin(), monthlyBinLowEdgeVector.end(), monthlyBinLowEdgeArray);

  // Create the 2D histogram
  int yAxisBinNumber = 20;
  TH2D candlePlot(plotName.c_str(), plotName.c_str(), monthlyBinLowEdgeVector.size()-1, monthlyBinLowEdgeArray, yAxisBinNumber, yAxisMin, yAxisMax);
  for (long t=0; t<sampleStepNumber; t++) {
    
    time_t currentTime = startTime + t*sampleStepSize;

    if ( daytimeOnly && !sun.isTimeDuringDay(currentTime) ) {
      continue;
    }

    double currentValue = ClimateFactory::instance()->getClimate()->getInterpolatedValue(valueName, currentTime);

    // Value may need some transformation
    currentValue = valueModificationFunction(currentValue);

    double rootTime = offsetToRootTime(currentTime);
    candlePlot.Fill(rootTime, currentValue);
  }

  // Clean up
  delete[] monthlyBinLowEdgeArray;


  return candlePlot;
}


template<typename T> void createCanvas(std::string canvasName,
				       std::vector<T> plots,
				       std::vector<std::string> options,
				       std::vector<int> colours,
				       std::string xAxisTitle,
				       std::string yAxisTitle) {

  int canvasWidth = 3500;
  int canvasHeight = 400;

  TCanvas canvas(canvasName.c_str(), "", canvasWidth, canvasHeight);
  plots[0].Draw(options[0].c_str());
  plots[0].SetTitle("");
  plots[0].GetXaxis()->SetTitle(xAxisTitle.c_str());
  plots[0].GetYaxis()->SetTitle(yAxisTitle.c_str());
  plots[0].GetXaxis()->SetTimeDisplay(1);
  plots[0].GetXaxis()->SetTimeFormat("%d/%m/%Y");
  plots[0].SetLineColor(colours[0]);

  for (unsigned int p=1; p<plots.size(); p++){
    plots[p].Draw(options[p].c_str());
    plots[p].SetLineColor(colours[p]);
  }

  canvas.Update();
  canvas.Write();

  // Dump out an eps as well
  canvas.SaveAs( (canvasName + ".eps").c_str() );
}

template<typename T> void createStackedCanvas(std::string canvasName,
					      std::vector<T> plots,
					      std::vector<std::string> options,
					      std::vector<int> colours,
					      std::string xAxisTitle,
					      std::vector<std::string> yAxisTitles) {

  int canvasWidth = 3500;
  int canvasHeightPerPlot = 300;

  if (plots.size() < 1) {
    return;
  }

  if (plots.size() != options.size() && 
      plots.size() != colours.size() && 
      plots.size() != yAxisTitles.size() ) {
    std::cerr << "Inconsistent input arrays, and so cannot create " << canvasName << std::endl;
    return;
  }

  int canvasHeight = canvasHeightPerPlot * plots.size();

  TCanvas canvas(canvasName.c_str(), "", canvasWidth, canvasHeight);

  float xMargin = 0.0;
  float yMargin = 0.0;
  canvas.Divide(1, plots.size(), xMargin, yMargin);

  std::vector<TGaxis*> temporaryAxis;

  // Draw axis for only a fraction of plot to avoid problems at the edge
  // of the stacked plot. Be careful not to go too big otherwise infinite loop!
  double axisPaddingFraction = 0.07;
  assert(axisPaddingFraction < 0.5);

  // Put a plot in each subdivision
  for (unsigned int p=0; p<plots.size(); p++) {
    auto pad = canvas.cd(p+1);

    pad->SetFrameBorderMode(0);
    pad->SetBorderMode(0);
    pad->SetBorderSize(0);

    plots[p].Draw(options[p].c_str());
    plots[p].SetTitle("");
    plots[p].SetLineColor(colours[p]);

    double absolutePaddingSize = std::fabs(plots[p].GetYaxis()->GetXmax() 
					   - plots[p].GetYaxis()->GetXmin()) * axisPaddingFraction;
    
    // Draw another y-axis on the right side as a test
    // Option 'S' means ticklength = fTickSize*axis_length
    TGaxis* yAxis = new TGaxis(plots[p].GetXaxis()->GetXmin(), plots[p].GetYaxis()->GetXmin() + absolutePaddingSize, 
			       plots[p].GetXaxis()->GetXmin(), plots[p].GetYaxis()->GetXmax() - absolutePaddingSize,
			       plots[p].GetYaxis()->GetXmin() + absolutePaddingSize, 
			       plots[p].GetYaxis()->GetXmax() - absolutePaddingSize, 
			       506, "S");
    yAxis->ImportAxisAttributes(plots[p].GetYaxis());
    yAxis->SetLineColor(kBlack);
    yAxis->SetTextColor(kBlack);
    yAxis->SetTitle(yAxisTitles[p].c_str());
    yAxis->CenterTitle(true);
    yAxis->Draw("");

    temporaryAxis.push_back(yAxis);
    
    // Draw another x-axis on the bottom pad plot only
    if ( p == plots.size()-1 ){ 

      absolutePaddingSize = std::fabs(plots[p].GetXaxis()->GetXmax() 
				      - plots[p].GetXaxis()->GetXmin()) * axisPaddingFraction;

      // Option 't' for time and 'S' for ticklength as above.
      TGaxis* xAxis = new TGaxis(plots[p].GetXaxis()->GetXmin() + absolutePaddingSize, plots[p].GetYaxis()->GetXmin(), 
				 plots[p].GetXaxis()->GetXmax() - absolutePaddingSize, plots[p].GetYaxis()->GetXmin(),
				 plots[p].GetXaxis()->GetXmin() + absolutePaddingSize, 
				 plots[p].GetXaxis()->GetXmax() - absolutePaddingSize, 
				 506, "tS");
      xAxis->ImportAxisAttributes(plots[p].GetXaxis());
      xAxis->SetLineColor(kBlack);
      xAxis->SetTextColor(kBlack);
      xAxis->SetTitle(xAxisTitle.c_str());
      xAxis->CenterTitle(false);

      TDatime da(1995,01,1,00,00,00);
      xAxis->SetTimeOffset(da.Convert());
      xAxis->SetTimeFormat("%d/%m/%Y");

      xAxis->Draw("");
      
      temporaryAxis.push_back(xAxis);

    }
  }

  canvas.Update();
  canvas.Write();

  // Dump out an eps as well
  canvas.SaveAs( (canvasName + ".eps").c_str() );

  // Clean up all the axis created
  for ( auto& axis : temporaryAxis ) {
    delete axis;
  }
  temporaryAxis.clear();
}

int main(int argc, char** argv) {

  bool daytimeOnly = true;

  GetOpt::GetOpt_pp ops(argc, argv);

  // Check for help request
  if (ops >> GetOpt::OptionPresent('h', "help")){
    showHelp();
    return 0;
  }

  ops >> GetOpt::Option("daytimeOnly", daytimeOnly, true);

  // Also do not run if other arguments are present
  if (ops.options_remain()) {
    std::cerr << "Oops! Unexpected options." << std::endl;
    showHelp();
    return -1;
  }

  std::cout << "Will " << (daytimeOnly ? "" : "not ") << "only use day time samples for candle plots" << std::endl;

  // Set the plotting style to 'flat' for the Solar Energy journal
  Style_SolarEnergyFlat();

  // For the interpolation plots lets setup some good time ranges
  time_t interpolationStartTime = getStartTime();
  time_t interpolationEndTime   = getEndTime();
  long   sampleStepSize = 10800; // 3 hours

  // Get the device location details
  LocationDetails deviceLocation("location.cfg");

  // Obtain the simulation sun
  Sun sun(deviceLocation);

  // Prepare the climate factory with the default configuration
  ClimateFactory::instance()->setConfigurationFile("default.cfg");
  ClimateFactory::instance()->setDeviceLocation(deviceLocation);

  // Get the raw data to plot
  std::vector<std::shared_ptr<const ClimateData>> extractedData = ClimateFactory::instance()->getClimate()->getData();

  // Make a graph of all the climate quantites
  TGraphAsymmErrors temperatureGraph;
  TGraphAsymmErrors totalColumnWaterGraph;
  TGraphAsymmErrors surfacePressureGraph;
  TGraphAsymmErrors totalCloudCoverGraph;
  TGraphAsymmErrors totalColumnOzoneGraph;

  std::string temperatureGraphName = "temperaturesOverYear";
  std::string totalColumnWaterGraphName = "totalColumnWaterOverYear";
  std::string surfacePressureGraphName = "surfacePressureOverYear";
  std::string totalCloudCoverGraphName = "totalCloudCoverOverYear";
  std::string totalColumnOzoneGraphName = "totalColumnOzoneOverYear";

  temperatureGraph.SetName(temperatureGraphName.c_str());
  totalColumnWaterGraph.SetName(totalColumnWaterGraphName.c_str());
  surfacePressureGraph.SetName(surfacePressureGraphName.c_str());
  totalCloudCoverGraph.SetName(totalCloudCoverGraphName.c_str());
  totalColumnOzoneGraph.SetName(totalColumnOzoneGraphName.c_str());

  // Add each time point
  for (unsigned int d=0; d<extractedData.size(); d++) {
    int nextPointIndex = temperatureGraph.GetN();

    long currentTime = offsetToRootTime(extractedData[d]->getTime());

    temperatureGraph.SetPoint(      nextPointIndex, currentTime, extractedData[d]->getValue("2 metre temperature")-CLHEP::STP_Temperature);
    totalColumnWaterGraph.SetPoint( nextPointIndex, currentTime, extractedData[d]->getValue("Total column water"));
    surfacePressureGraph.SetPoint(  nextPointIndex, currentTime, extractedData[d]->getValue("Surface pressure")/1000.0);
    totalCloudCoverGraph.SetPoint(  nextPointIndex, currentTime, extractedData[d]->getValue("Total cloud cover"));
    totalColumnOzoneGraph.SetPoint( nextPointIndex, currentTime, extractedData[d]->getValue("Total column ozone")*1000.0);
  }

  // Construct some interpolation graphs
  TGraphAsymmErrors temperatureInterpolGraph = createInterpolationGraph("interpolatedTemperature", 
									interpolationStartTime, 
									interpolationEndTime, 
									sampleStepSize, 
									"2 metre temperature", 
									[](double v) { return v - CLHEP::STP_Temperature; } );

  TGraphAsymmErrors totalColumnWaterInterpolGraph = createInterpolationGraph("interpolatedTotalColumnWater", 
									     interpolationStartTime, 
									     interpolationEndTime, 
									     sampleStepSize, 
									     "Total column water");

  TGraphAsymmErrors surfacePressureInterpolGraph = createInterpolationGraph("interpolatedSurfacePressure", 
									     interpolationStartTime, 
									     interpolationEndTime, 
									     sampleStepSize, 
									    "Surface pressure",
									    [](double v) { return v/1000.0; });

  TGraphAsymmErrors totalCloudCoverInterpolGraph = createInterpolationGraph("interpolatedTotalCloudCover", 
									    interpolationStartTime, 
									    interpolationEndTime, 
									    sampleStepSize, 
									    "Total cloud cover");

  TGraphAsymmErrors totalColumnOzoneInterpolGraph = createInterpolationGraph("interpolatedTotalColumnOzone", 
									     interpolationStartTime, 
									     interpolationEndTime, 
									     sampleStepSize, 
									     "Total column ozone",
									     [](double v) { return v*1000.0; });
  // Create some candle plots
  TH2D temperatureCandlePlot = createCandlePlot("candleTemperature",
						interpolationStartTime,
						interpolationEndTime,
						sampleStepSize,
						"2 metre temperature",
						daytimeOnly,
						sun,
						[](double v) { return v - CLHEP::STP_Temperature; });

  TH2D totalColumnWaterCandlePlot = createCandlePlot("candleTotalColumnWater",
						     interpolationStartTime,
						     interpolationEndTime,
						     sampleStepSize,
						     "Total column water",
						     daytimeOnly,
						     sun);

  TH2D surfacePressureCandlePlot = createCandlePlot("candleSurfacePressure",
						    interpolationStartTime,
						    interpolationEndTime,
						    sampleStepSize,
						    "Surface pressure",
						    daytimeOnly,
						    sun,
						    [](double v) { return v/1000.0; });

  TH2D totalCloudCoverCandlePlot = createCandlePlot("candleTotalCloudCover",
						    interpolationStartTime,
						    interpolationEndTime,
						    sampleStepSize,
						    "Total cloud cover",
						    daytimeOnly,
						    sun);

  TH2D totalColumnOzoneCandlePlot = createCandlePlot("candleTotalColumnOzone",
						     interpolationStartTime,
						     interpolationEndTime,
						     sampleStepSize,
						     "Total column ozone",
						     daytimeOnly,
						     sun,
						     [](double v) { return v*1000.0; });


  gStyle->SetPaperSize(200,260);
  
  // Save the graphs to disk and export to eps files
  TFile resultsFile("climateTesting.root", "RECREATE");

  createCanvas<TGraphAsymmErrors>("temperatureOverYearCanvas",
    {temperatureGraph, temperatureInterpolGraph},
    {"AL","SAMEL"},
    {kBlack,kRed},
    "Date",
    "T_{2m} [#circC]");

  createCanvas<TGraphAsymmErrors>("totalColumnWaterOverYearCanvas",
    {totalColumnWaterGraph, totalColumnWaterInterpolGraph},
    {"AL","SAMEL"},
    {kBlack,kRed},
    "Date",
    "TCW [kgm^{-2}]");

  createCanvas<TGraphAsymmErrors>("surfacePressureOverYearCanvas",
    {surfacePressureGraph, surfacePressureInterpolGraph},
    {"AL","SAMEL"},
    {kBlack,kRed},
    "Date",
    "P_{surface} [kPa]");

  createCanvas<TGraphAsymmErrors>("totalCloudCoverOverYearCanvas",
    {totalCloudCoverGraph, totalCloudCoverInterpolGraph},
    {"AL","SAMEL"},
    {kBlack,kRed},
    "Date",
    "C_{total} [0->1]");

  createCanvas<TGraphAsymmErrors>("totalColumnOzoneOverYearCanvas",
    {totalColumnOzoneGraph, totalColumnOzoneInterpolGraph},
    {"AL","SAMEL"},
    {kBlack,kRed},
    "Date",
    "TCO [kgm^{-2}]");

  // Make some canvas versions of the candle plots
  createCanvas<TH2D>("candleTemperatureCanvas",
    {temperatureCandlePlot},
    {"CANDLE"},
    {kBlack},
    "Date",
    "T_{2m} [#circC]");

  createCanvas<TH2D>("candleTotalColumnWaterCanvas",
    {totalColumnWaterCandlePlot},
    {"CANDLE"},
    {kBlack},
    "Date",
    "TCW [kgm^{-2}]");

  createCanvas<TH2D>("candleSurfacePressureCanvas",
    {surfacePressureCandlePlot},
    {"CANDLE"},
    {kBlack},
    "Date",
    "P_{surface} [Pa]");

  createCanvas<TH2D>("candleTotalCloudCoverCanvas",
    {totalCloudCoverCandlePlot},
    {"CANDLE"},
    {kBlack},
    "Date",
    "C_{total} [0->1]");

  createCanvas<TH2D>("candleTotalColumnOzoneCanvas",
    {totalColumnOzoneCandlePlot},
    {"CANDLE"},
    {kBlack},
    "Date",
    "TCO [gm^{-2}]");

  // Also put all the plots stacked on the same canvas
  // This will save space in the paper if necessary.
  createStackedCanvas<TH2D>("stackedCandleClimateCanvas",
    {temperatureCandlePlot, totalColumnWaterCandlePlot, surfacePressureCandlePlot, totalCloudCoverCandlePlot, totalColumnOzoneCandlePlot},
    {"A CANDLE", "A CANDLE", "A CANDLE", "A CANDLE", "A CANDLE"},
    {kBlack, kBlack, kBlack, kBlack, kBlack},
    "Date",
    {"T_{2m} [#circC]", "TCW [kgm^{-2}]", "P_{surface} [kPa]", "C_{total} [0->1]", "TCO [gm^{-2}]"});

  resultsFile.Close();

  return 0;
}

