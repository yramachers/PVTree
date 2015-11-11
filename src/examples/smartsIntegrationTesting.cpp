/*! @file
 * \brief Testing of SMARTS integration using
 *        new spectrum factory. 
 *
 */
#include "solarSimulation/spectrumFactory.hpp"
#include "location/locationDetails.hpp"

#include <iostream>
#include <cstring>
#include <algorithm>
#include <sstream>
#include <iomanip>

#include <CLHEP/Units/SystemOfUnits.h>

// save diagnostic state
#pragma GCC diagnostic push 

// turn off the specific warning.
#pragma GCC diagnostic ignored "-Wshadow"

#include "TFile.h"
#include "TAxis.h"
#include "TCanvas.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TLegend.h"
#include "TColor.h"

// turn the warnings back on
#pragma GCC diagnostic pop

using CLHEP::gram;
using CLHEP::cm2;
using CLHEP::kilogram;
using CLHEP::m2;

void createCanvas(std::string canvasName,
		  std::string canvasTitle,
		  std::vector<std::shared_ptr<TH1D>> histograms,
		  std::string xAxisName,
		  std::string yAxisName,
		  std::vector<int> colours,
		  std::vector<double> histogramSettings,
		  std::string settingUnits) {

  TCanvas canvas(canvasName.c_str(), canvasTitle.c_str());
  
  histograms[0]->Draw();
  histograms[0]->GetXaxis()->SetTitle(xAxisName.c_str());
  histograms[0]->GetYaxis()->SetTitle(yAxisName.c_str());
  histograms[0]->SetTitle(canvasTitle.c_str());

  // Loop over remaining histograms
  for (unsigned int h=0; h<histograms.size(); h++) {
    histograms[h]->Draw("SAMEHIST");
    histograms[h]->SetLineColor(colours[h]);
    histograms[h]->SetMarkerColor(colours[h]);
  }

  // Set the y axis to a reasonable range.
  histograms[0]->GetYaxis()->SetRangeUser(0.00001, 20.0);

  // Create a legend
  TLegend legend(0.7, 0.1, .9, .3);

  for (unsigned int h=0; h<histograms.size(); h++) {
    std::stringstream histogramName;
    histogramName << std::setprecision(3) << histogramSettings[h] << " " << settingUnits;

    legend.AddEntry(histograms[h].get(), histogramName.str().c_str(), "l"); 
  }
  
  legend.Draw();

  canvas.SetLogy(1);
  canvas.Update();
  canvas.Write();
}

void createProjectedCanvas(std::string canvasName,
			   std::string canvasTitle,
			   TH2D& histogram,
			   std::string xAxisName,
			   std::string yAxisName) {

  TCanvas canvas(canvasName.c_str(), canvasTitle.c_str());

  histogram.Draw("AITOFF");
  histogram.GetXaxis()->SetTitle(xAxisName.c_str());
  histogram.GetYaxis()->SetTitle(yAxisName.c_str());
  histogram.SetTitle(canvasTitle.c_str());

  canvas.Update();
  canvas.Write();
}


int main() {
  
  // Get the device location details
  LocationDetails deviceLocation("location.cfg");

  // Get the spectrum factory
  SpectrumFactory* factory = SpectrumFactory::instance();

  // Set the altitude for specific device location
  factory->setAltitude(deviceLocation.getAltitude());

  // Produce the default spectrum
  std::cout << "Running smarts." << std::endl;
  std::shared_ptr<Spectrum> spectrum = factory->getSpectrum();
  std::cout << "Finished running smarts." << std::endl;

  // Now try iterating over a number of solar elevations to make a 
  // nice comparison plot of the direct irradiance (over a day).
  std::vector<double> elevations = { 0, 5, 10, 20, 40, 60 };
  std::vector<std::shared_ptr<TH1D> > spectralHistograms;
  std::vector<int> colours = { 40, 41, 42, 43, 44, 45, 46, 47, 48};

  for (double elevation : elevations) {
    // Regenerate spectrum at new elevation
    factory->setSolarPositionWithElevationAzimuth((double)elevation, 0.0);
    spectrum = factory->getSpectrum();

    // Get a copy of the normal irradiance histogram
    spectralHistograms.push_back( spectrum->getHistogram("Direct_normal_irradiance") );
  }

  // For remaining spectrum set a single elevtation to ...
  factory->setSolarPositionWithElevationAzimuth(50.0, 0.0);

  // Try also varying the amount of water in the column
  std::vector<double> waterColumnDensities = { 3, 6, 12, 24, 48 }; // kg/m^2
  std::vector<std::shared_ptr<TH1D> > waterVariationHistograms;
  
  for (double waterColumnDensity : waterColumnDensities) {

    double precipitableWater = waterColumnDensity / ( (gram/cm2) * (m2/kilogram) ); // convert to g/cm^2
    factory->setPrecipitableWater( precipitableWater );
    spectrum = factory->getSpectrum();

    // Get a copy of the normal irradiance histogram
    waterVariationHistograms.push_back( spectrum->getHistogram("Direct_normal_irradiance") );
  }

  // Return to the default for next variation.
  factory->setDefaultPrecipitableWater(); 

  // Try also varying the pressure
  std::vector<double> atmosphericPressures =  {950.0, 970.0, 990.0, 1010.0, 1030.0}; // hPa (or mb...)
  std::vector<std::shared_ptr<TH1D> > pressureVariationHistograms;
  
  for (double pressure : atmosphericPressures) {

    factory->setAtmosphericPressure(pressure);
    spectrum = factory->getSpectrum();

    // Get a copy of the normal irradiance histogram
    pressureVariationHistograms.push_back( spectrum->getHistogram("Direct_normal_irradiance") );
  }

  // Return to the default for the next variation
  factory->setDefaultAtmosphericPressure();
  
  // Try also varying the ozone abundance
  std::vector<double> ozoneDensities = { 0.005, 0.007, 0.009, 0.01, 0.012 }; // kg m^{-2}
  std::vector<double> ozoneAbundances; // atm-cm
  std::vector<std::shared_ptr<TH1D> > ozoneAbundanceHistograms;

  for (double ozoneDensity : ozoneDensities) {

    // Convert to atm-cm
    double oxygen3Mass = 3.0*2.6568e-26; // kg
    double loschmidtConstant = 2.6868e25; // m-3
    double abundance = ( (ozoneDensity/oxygen3Mass)/loschmidtConstant )*100.0;
    ozoneAbundances.push_back(abundance);

    factory->setOzoneAbundance(abundance);
    spectrum = factory->getSpectrum();

    // Get a copy of the normal irradiance histogram
    ozoneAbundanceHistograms.push_back( spectrum->getHistogram("Direct_normal_irradiance") );
  }

  // Return to the default
  factory->setDefaultOzoneAbundance();

  // Try varying the temperature
  std::vector<double> temperatures = {-20.0, -10.0, 0.0, 10.0, 20.0, 30.0}; // degrees C
  std::vector<std::shared_ptr<TH1D> > temperatureHistograms;

  for (double temperature : temperatures) {
    double fixedHumidity = 20.0; // %
    time_t fixedTime = 0; // doesn't do anything at the moment
    double fixedOzoneAbundance = 0.4; // atm-cm
    double fixedPrecipitableWater = 12.0 / ( (gram/cm2) * (m2/kilogram) ); // g/cm^2
    SpectrumFactory::GasLoad fixedGasLoad = SpectrumFactory::MODERATE_POLLUTION; 

    factory->setOzoneAbundance(fixedOzoneAbundance);
    factory->setPrecipitableWater(fixedPrecipitableWater);
    factory->setGasLoad(fixedGasLoad);
    factory->setAtmosphereProperties(temperature, fixedHumidity, fixedTime, temperature);
    spectrum = factory->getSpectrum();

    // Get a copy of the normal irradiance histogram
    temperatureHistograms.push_back( spectrum->getHistogram("Direct_normal_irradiance") );
  }

  // Try varying the relative humidity
  std::vector<double> relativeHumidities = {0.0, 20.0, 40.0, 60.0, 80.0, 100.0}; // %
  std::vector<std::shared_ptr<TH1D> > humidityHistograms;

  for (double humidity : relativeHumidities) {
    double fixedTemperature = 18; // C
    time_t fixedTime = 0; // doesn't do anything at the moment
    double fixedOzoneAbundance = 0.4; // atm-cm
    double fixedPrecipitableWater = 12.0 / ( (gram/cm2) * (m2/kilogram) ); // g/cm^2
    SpectrumFactory::GasLoad fixedGasLoad = SpectrumFactory::MODERATE_POLLUTION; 

    factory->setOzoneAbundance(fixedOzoneAbundance);
    factory->setPrecipitableWater(fixedPrecipitableWater);
    factory->setGasLoad(fixedGasLoad);
    factory->setAtmosphereProperties(fixedTemperature, humidity, fixedTime, fixedTemperature);
    spectrum = factory->getSpectrum();

    // Get a copy of the normal irradiance histogram
    humidityHistograms.push_back( spectrum->getHistogram("Direct_normal_irradiance") );
  }

  // Return to the default
  factory->setDefaultOzoneAbundance();
  factory->setDefaultAtmosphereProperties();
  factory->setDefaultPrecipitableWater(); 
  factory->setDefaultGasLoad();

  // Vary the pollution level
  std::vector<double > pollutionLevels = { 1, 2, 3, 4 }; // for plotting
  std::vector<SpectrumFactory::GasLoad > pollutionSelections = {SpectrumFactory::PRISTINE,
								SpectrumFactory::LIGHT_POLLUTION,
								SpectrumFactory::MODERATE_POLLUTION,
								SpectrumFactory::SEVERE_POLLUTION};
  std::vector<std::shared_ptr<TH1D> > pollutionHistograms;
  
  for (auto pollutionSelection : pollutionSelections) {

    factory->setGasLoad(pollutionSelection);
    spectrum = factory->getSpectrum();

    // Get a copy of the normal irradiacnce histogram
    pollutionHistograms.push_back( spectrum->getHistogram("Direct_normal_irradiance") );
  }

  // Return to the default
  factory->setDefaultGasLoad();

  // Test tilted diffuse spectra evaluation
  std::vector<double > tiltedAzimuthValues = {0.0, 45.0, 90.0, 135.0, 180.0};
  double tiltedElevation = 45.0;
  int    tiltedAlbedoIndex = 38;
  int    tiltedDirectOutputVariable = 6;
  int    tiltedDiffuseOutputVariable = 7;
  factory->setTiltLocalAlbedo(tiltedAlbedoIndex);
  factory->appendOutputVariable(tiltedDirectOutputVariable);
  factory->appendOutputVariable(tiltedDiffuseOutputVariable);

  std::vector<std::shared_ptr<TH1D> > tiltedDirectHistograms;
  std::vector<std::shared_ptr<TH1D> > tiltedDiffuseHistograms;

  for (double tiltedAzimuth : tiltedAzimuthValues) {
    factory->setTiltAngles(tiltedElevation, tiltedAzimuth);
    spectrum = factory->getSpectrum();

    // Get a copy of the tilted direct and diffuse histograms
    tiltedDirectHistograms.push_back( spectrum->getHistogram("Direct_tilted_irradiance") );
    tiltedDiffuseHistograms.push_back( spectrum->getHistogram("Difuse_tilted_irradiance") );
  }
  
  // Also look at the total integrated irradiance across the sky
  tiltedAzimuthValues.clear();
  for (unsigned int az=0; az<36; az++) {
    tiltedAzimuthValues.push_back(az*10.0 + 5.0);
  }
  std::vector<double > tiltedElevationValues;
  for (unsigned int el=0; el<9; el++) {
    tiltedElevationValues.push_back(el*10.0 + 5.0);
  }  
  TH2D diffuseSkyTotalIrradiance("DiffuseSkyTotalIrradiance","Aitoff",tiltedAzimuthValues.size(), -180.0, 180.0, tiltedElevationValues.size(), 0.0, 90.0);

  for (double tiltedAzimuth : tiltedAzimuthValues) {
    for (double tiltedElevationValue : tiltedElevationValues) {
      factory->setTiltAngles(tiltedElevationValue, tiltedAzimuth);
      spectrum = factory->getSpectrum();

      double diffuseIrradianceSum = spectrum->getHistogram("Difuse_tilted_irradiance")->Integral("width");
      diffuseSkyTotalIrradiance.Fill(tiltedAzimuth-180.0,tiltedElevationValue, diffuseIrradianceSum);
    }
  }


  // Combine all the plots together on a canvas and save to ROOT file.
  TFile smartsResults("smarts.results.root", "RECREATE");

  // Make the elevation variation canvas
  createCanvas("IrradianceVsElevation",
	       "Irradiance Vs Elevation",
	       spectralHistograms,
	       "Wavelength [nm]",
	       "Irradiance [Wm^{-2}]",
	       colours,
	       elevations,
	       "degrees");

  // Make the precipitable water variation canvas
  createCanvas("IrradianceVsPrecipitableWater",
	       "Irradiance Vs Precipitable Water",
	       waterVariationHistograms,
	       "Wavelength [nm]",
	       "Irradiance [Wm^{-2}]",
	       colours,
	       waterColumnDensities,
	       "kgm^{-2}");

  // Make the atmospheric pressure variation canvas
  createCanvas("IrradianceVsAtmosphericPressure",
	       "Irradiance Vs Atmospheric Pressure",
	       pressureVariationHistograms,
	       "Wavelength [nm]",
	       "Irradiance [Wm^{-2}]",
	       colours,
	       atmosphericPressures,
	       "hPa");

  // Make the ozone abundance canvas
  createCanvas("IrradianceVsOzoneAbundance",
	       "Irradiance Vs O_{3} Abundance",
	       ozoneAbundanceHistograms,
	       "Wavelength [nm]",
	       "Irradiance [Wm^{-2}]",
	       colours,
	       ozoneAbundances,
	       "atm-cm");

  // Make the air temperature canvas
  createCanvas("IrradianceVsAirTemperature",
	       "Irradiance Vs Air Temperature",
	       temperatureHistograms,
	       "Wavelength [nm]",
	       "Irradiance [Wm^{-2}]",
	       colours,
	       temperatures,
	       " degrees C");

  // Make the relative humidity canvas
  createCanvas("IrradianceVsRelativeHumidity",
	       "Irradiance Vs Relative Humidity",
	       humidityHistograms,
	       "Wavelength [nm]",
	       "Irradiance [Wm^{-2}]",
	       colours,
	       relativeHumidities,
	       " %");

  // Make the pollution variation canvas
  createCanvas("IrradianceVsPollution",
	       "Irradiance Vs Pollution",
	       pollutionHistograms,
	       "Wavelength [nm]",
	       "Irradiance [Wm^{-2}]",
	       colours,
	       pollutionLevels,
	       "");

  // Make the direct tilted irradiance variation canvas
  createCanvas("DirectIrradianceVsSurfaceAzimuth",
	       "Direct Tilted Irradiance Vs Azimuth",
	       tiltedDirectHistograms,
	       "Wavelength [nm]",
	       "Direct Tilted Irradiance [Wm^{-2}]",
	       colours,
	       tiltedAzimuthValues,
	       " deg");

  // Make the diffuse tilted irradiance variation canvas
  createCanvas("DiffuseIrradianceVsSurfaceAzimuth",
	       "Diffuse Tilted Irradiance Vs Azimuth",
	       tiltedDiffuseHistograms,
	       "Wavelength [nm]",
	       "Diffuse Tilted Irradiance [Wm^{-2}]",
	       colours,
	       tiltedAzimuthValues,
	       " deg");

  // Draw the projection
  createProjectedCanvas("DiffuseTiltedAitoff",
			"Aitoff projection of total irradiance",
			diffuseSkyTotalIrradiance,
			"Azimuth",
		        "Elevation");

  smartsResults.Close();
}
