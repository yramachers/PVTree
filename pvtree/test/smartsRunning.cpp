#include "pvtree/test/catch.hpp"
#include "pvtree/full/solarSimulation/spectrumFactory.hpp"
#include "pvtree/location/locationDetails.hpp"

TEST_CASE( "solarSimulation/spectrumFactory", "[sun]" ) {

  // Get the device location details
  LocationDetails deviceLocation("location.cfg");

  // Get the spectrum factory
  SpectrumFactory* factory = SpectrumFactory::instance();
  factory->setDefaults();
  factory->setAltitude(deviceLocation.getAltitude());

  // Produce the default spectrum online with SMARTS
  std::shared_ptr<Spectrum> spectrum = factory->getSpectrum();

  // Get validation spectrum from file
  Spectrum validationSpectrum("spectra/validation.default.results");

  // Check equality of all data as well
  CHECK( *(spectrum.get()) == validationSpectrum );

  // Produce a second spectrum online with the same parameters
  // so it should be identical as well. Need to clear the factory
  // cache to force SMARTS to actually re-run.
  factory->clearCache();
  std::shared_ptr<Spectrum> secondSpectrum = factory->getSpectrum();

  CHECK( *(spectrum.get()) == *(secondSpectrum.get()) );

  // For sanity purposes also should equal self
  CHECK( *(spectrum.get()) == *(spectrum.get()) );

  // If I change the parameters in a signficant way do I get
  // different spectra?
  double elevation = 10.0;
  factory->setSolarPositionWithElevationAzimuth(elevation, 0.0);
  std::shared_ptr<Spectrum> lowElevationSpectrum = factory->getSpectrum();

  elevation = 60.0;
  factory->setSolarPositionWithElevationAzimuth(elevation, 0.0);
  std::shared_ptr<Spectrum> highElevationSpectrum = factory->getSpectrum();

  CHECK(  *(lowElevationSpectrum.get()) != *(highElevationSpectrum.get()) );
}
