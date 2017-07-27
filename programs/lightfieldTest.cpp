/*! @file lightfieldTest.cpp
 * \brief Testing ideas for using a lightfield (-lite) 
 *
 * In reality it is just a test of the use of an approximation
 * of the 2d plenoptic function ( theta, E ).
 */

#include "pvtree/full/solarSimulation/plenoptic1D.hpp"

#include <vector>
#include <cmath>
#include <iomanip>

// save diagnostic state
#pragma GCC diagnostic push 

// turn off the specific warning.
#pragma GCC diagnostic ignored "-Wshadow"

#include "TH1D.h"
#include "TH2D.h"
#include "TF1.h"
#include "TFile.h"
#include "TROOT.h"
#include "TRandom.h"
#include "TVector3.h"
#include "TCanvas.h"
#include "TLine.h"

// turn the warnings back on
#pragma GCC diagnostic pop


/*! \brief Generate a 'photon' from a test spectrum
 *
 * @param[in] number The total number of photons to generate.
 * @param[in] angle The direction of travel in radians.
 *
 * \returns A vector of 'photon energies' and weights
 */
std::vector<std::tuple<double,double> > generateTestPhotons(unsigned int number, double angle) {

  // Create a static histogram to represent a spectrum to be
  // used.
  static bool isSpectrumConstructed = false;
  static TH1D spectrum("GeneratedPhotonSpectrum","GeneratedPhotonSpectrum", 25, 0.0, 100.0);
  static double spectralSum = 0.0;
  static int spectrumGenerationNumber = 500000; //should be reasonably smooth

  if (!isSpectrumConstructed) {

    TF1* gaussianDistribution = static_cast<TF1*>( gROOT->GetFunction("gaus") );
    gaussianDistribution->SetParameter(1, 15.0); // shift the mean.
    gaussianDistribution->SetParameter(2, 40.0); // increase standard deviation.

    // Randomly fill the histogram with a gaussian
    spectrum.FillRandom("gaus", spectrumGenerationNumber);

    // Also calculate the sum
    spectralSum = spectrum.Integral("width");

    // Save it to the results file for checking
    spectrum.Write();

    isSpectrumConstructed = true;
  }

  double scaleFactor = spectralSum/number;

  // Apply an angle dependant scaling as well
  angle = std::remainder(angle, 2.0*M_PI);
  /*
  double minimumActiveAngle = 0.0*M_PI;
  double maximumActiveAngle = 2.0*M_PI;
  
  if ( angle < minimumActiveAngle || angle > maximumActiveAngle ){
    // Not in required range
    scaleFactor = 0.0;
  } else {
    scaleFactor *= std::sin( M_PI * (angle - minimumActiveAngle)/(maximumActiveAngle - minimumActiveAngle) );
  }
  */
  std::vector<std::tuple<double,double> > testPhotons;
  
  for (unsigned int n=0; n<number; n++){
    testPhotons.push_back( std::make_tuple(spectrum.GetRandom(), scaleFactor) );
  }
  
  return testPhotons;
}

/*! \brief From a vector of tuples create a 2d histogram for testing convergence
 *
 * @param[in] plotName Name of the histogram ROOT should create and save to disk.
 * @param[in] irradiances The vector of 2d tuples which contains all the entries.
 * @param[in] xAxisBinCentres The set of bin centres to use for the x axis.
 */
void createConvergencePlot(std::string plotName, 
			   std::vector<std::tuple<double, int> >& irradiances, 
			   std::vector<int> xAxisBinCentres) {
  
  // Evaluate reasonable binning
  // x-axis
  std::vector<double> binWidths;
  std::vector<double> binLowEdges;
  binWidths.push_back( xAxisBinCentres[1] - xAxisBinCentres[0] );
  binLowEdges.push_back( xAxisBinCentres[0] - binWidths[0]/2.0 );

  for (unsigned int b=1; b<xAxisBinCentres.size(); b++) {
    
    double distanceToPreviousBinCentre = xAxisBinCentres[b]-xAxisBinCentres[b-1];
    
    double currentBinHalfWidth = distanceToPreviousBinCentre - (binWidths.back()/2.0);

    if (currentBinHalfWidth < 0.0) {
      std::cerr << "Can't use negative bin widths. Logic problem in irradiance histogram creation!" << std::endl;
      throw;
    }
    
    binWidths.push_back( 2.0*currentBinHalfWidth );
    binLowEdges.push_back( xAxisBinCentres[b]-binWidths[b]/2.0 );
  }

  // Add on the upper edge of the final bin
  binLowEdges.push_back(binLowEdges.back() + binWidths.back());

  double* xAxisLowEdgeArray = new double[binLowEdges.size()];
  std::copy(binLowEdges.begin(), binLowEdges.end(), xAxisLowEdgeArray);

  // y-axis
  int yAxisBinNumber = 500;
  bool initalYValue = true;
  double minSummedIrradiance = 0.0;
  double maxSummedIrradiance = 0.0;
  for ( auto& irradiance : irradiances ) {

    if (initalYValue) {
      minSummedIrradiance = std::get<0>(irradiance);
      maxSummedIrradiance = std::get<0>(irradiance);
      initalYValue = false;
      continue;
    }

    if ( minSummedIrradiance > std::get<0>(irradiance) ) {
         minSummedIrradiance = std::get<0>(irradiance);
    }
    if ( maxSummedIrradiance < std::get<0>(irradiance) ) {
         maxSummedIrradiance = std::get<0>(irradiance);
    }

  }

  // pad the y-axis a little
  double yAxisPaddingFraction = 0.1;
  double yAxisPadding = yAxisPaddingFraction * std::fabs(maxSummedIrradiance - minSummedIrradiance);
  minSummedIrradiance -= yAxisPadding;
  maxSummedIrradiance += yAxisPadding;

  TH2D candlePlot(plotName.c_str(), plotName.c_str(), binLowEdges.size()-1, xAxisLowEdgeArray, yAxisBinNumber, minSummedIrradiance, maxSummedIrradiance);

  // Fill the histogram
  for ( auto& irradiance : irradiances ) {
    candlePlot.Fill( std::get<1>(irradiance), std::get<0>(irradiance) );
  }

  candlePlot.Write();

  delete[] xAxisLowEdgeArray;
}

void fillPlenopticFunction(Plenoptic1D& plenopticFunction, double sourceMinimumAngle, double sourceMaximumAngle, int photonNumberPerSample, int stepNumber) {

  double angleStep = (sourceMaximumAngle-sourceMinimumAngle)/stepNumber;
      
  for (int s=0; s<stepNumber; s++) {
    double currentAngle = sourceMinimumAngle + angleStep*s + angleStep*0.5;
	
    std::vector<std::tuple<double, double> > photons = generateTestPhotons(photonNumberPerSample, currentAngle);
	
    double energySum = 0.0;
    for (auto& photon : photons) {
      energySum += std::get<0>(photon) * std::get<1>(photon);
    }
	
    // the size of the 'time step'.
    energySum *= angleStep; 

    plenopticFunction.fill(currentAngle, energySum);
  }
}

double Cross2D(TVector2 v, TVector2 w) {
  return v.X()*w.Y() - v.Y()*w.X();
}

double Dot2D(TVector2 v, TVector2 w) {
  return v.X()*w.X() + v.Y()*w.Y();
}

/*! \brief Check for intersection of two line segment.
 *
 * Using example given in 
 * http://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
 */
bool isIntersecting(TVector2 p, TVector2 r, TVector2 q, TVector2 s) {

  if ( Cross2D(r,s) == 0 && Cross2D((q-p),r) != 0) {
    // parallel and non-intersecting
    return false;
  }

  if ( Cross2D(r,s) == 0 && Cross2D((q-p),r) == 0) {
    // The lines are collinear, check for overlap
    double t0 = Dot2D((q - p),r)/Dot2D(r,r);
    double t1 = t0 + Dot2D(s,r)/Dot2D(r,r);

    if (t0 > 1.0 && t1 > 1.0) {
      return false;
    }

    if (t0 < 0.0 && t1 < 0.0) {
      return false;
    }

    return true;
  }

  // Calculate t and u
  double t = Cross2D((q-p),s) / Cross2D(r,s);
  double u = Cross2D((q-p),r) / Cross2D(r,s);

  if ( Cross2D(r,s) != 0 &&
       t >= 0.0 && t <= 1.0  &&
       u >= 0.0 && u <= 1.0) {
    // They intersect within each of the segments
    return true;
  }

  // Not parallel but do not intersect
  return false;
}


double testGenerationConvergence(std::string testName, double targetOrientationAngle, double sourceMinimumAngle, double sourceMaximumAngle, int photonNumberPerSample, int seedOffset) {

  std::vector<int> sourceStepNumbers =       {10,  20,  50,  100};
  std::vector<unsigned int> sourceAttempts = {100, 100, 100, 100};
  std::vector<std::tuple<double, int> > targetIrradiances;

  if ( sourceStepNumbers.size() != sourceAttempts.size() ){
    std::cerr << "Inconsistent vector sizes." << std::endl;
    throw;
  }

  // Check that the angular integration does converge
  for (unsigned int i=0; i<sourceStepNumbers.size(); i++) {
    int stepNumber = sourceStepNumbers[i];

    // Set the random number seed
    gRandom->SetSeed(i+seedOffset);

    for (unsigned int a=0; a<sourceAttempts[i]; a++) {
    
      double irradianceSum = 0.0;
      double angleStep = (sourceMaximumAngle-sourceMinimumAngle)/stepNumber;
      
      for (int s=0; s<stepNumber; s++) {
	double currentAngle = sourceMinimumAngle + angleStep*s + angleStep*0.5;
	
	std::vector<std::tuple<double, double> > photons = generateTestPhotons(photonNumberPerSample, currentAngle*(M_PI/180.0));
	
	double energySum = 0.0;
	for (auto& photon : photons) {
	  energySum += std::get<0>(photon) * std::get<1>(photon);
	}
	
	irradianceSum += energySum * (M_PI/180.0) * angleStep * std::fabs( std::cos( (targetOrientationAngle - 90.0 - currentAngle ) * (M_PI/180.0) ) );
      }
      
      targetIrradiances.push_back( std::make_tuple(irradianceSum, stepNumber) );

    }
  }

  // Show convergence on a 2d plot
  createConvergencePlot( (testName+std::string("Convergence")).c_str(), targetIrradiances, sourceStepNumbers);

  // Get the average irradiance from the final step
  double averageIrradiance = 0.0;

  for ( auto& irradiance : targetIrradiances ) {
    if ( std::get<1>(irradiance) == sourceStepNumbers.back() ) {
      averageIrradiance += std::get<0>(irradiance)/sourceAttempts.back();
    }
  }

  return averageIrradiance;
}

void testPlenopticFunction(std::string testName, double targetOrientationAngle, double sourceMinimumAngle, double sourceMaximumAngle, int photonNumberPerSample) {

  std::cout << "Evaluating " << testName << ": -"<< std::endl;

  // Just stick with a given seed
  int seedOffset = 1234;

  // Check generic convergence of the generator.
  double bestAverageConvergence = testGenerationConvergence(testName, 
							    targetOrientationAngle, 
							    sourceMinimumAngle, 
							    sourceMaximumAngle, 
							    photonNumberPerSample, 
							    seedOffset);

  std::cout << std::setw(19) << "Old way sum = " << bestAverageConvergence << std::endl;

  // Attempt to create + test plenoptic functions with regular grid spacing
  double regularMin = 0.0;
  double regularMax = 360.0;
  unsigned int regularBinNumber = 720;
  double regularBinSize = std::fabs(regularMax - regularMin)/regularBinNumber;
  std::vector<double> regularAngularGrid;

  for (unsigned int b=0; b<regularBinNumber; b++) {
    regularAngularGrid.push_back( (regularMin + regularBinSize*b) * (M_PI/180.0) );
  }

  Plenoptic1D regularPlenopticFunction(regularAngularGrid);

  // fill the plenoptic function
  fillPlenopticFunction(regularPlenopticFunction, 
			sourceMinimumAngle * (M_PI/180.0), 
			sourceMaximumAngle * (M_PI/180.0), 
			photonNumberPerSample, 
			2000); // step number

  // sample the plenoptic function using a specific structure
  std::vector<std::tuple<double, double> > surfaceGeometry;

  surfaceGeometry.push_back( std::make_tuple(  5.0, -5.0 ) );
  surfaceGeometry.push_back( std::make_tuple(  8.0,  0.0 ) );
  surfaceGeometry.push_back( std::make_tuple(  5.0,  5.0 ) );
  surfaceGeometry.push_back( std::make_tuple(  0.0,  8.0 ) );
  surfaceGeometry.push_back( std::make_tuple( -5.0,  5.0 ) );
  surfaceGeometry.push_back( std::make_tuple( -8.0,  0.0 ) );
  surfaceGeometry.push_back( std::make_tuple( -5.0, -5.0 ) );
  surfaceGeometry.push_back( std::make_tuple(  0.0, -8.0 ) );
  surfaceGeometry.push_back( std::make_tuple(  5.0, -5.0 ) );

  regularPlenopticFunction.setSurfaceGeometry(surfaceGeometry);
  auto particles = regularPlenopticFunction.generate(200000);

  //Check if each particles interacts with 'sensitive detector'
  TVector2 sensitiveStartingPoint(-0.5*std::sin(targetOrientationAngle*(M_PI/180.0)), -0.5*std::cos(targetOrientationAngle*(M_PI/180.0)) );
  TVector2 sensitiveTrajectory(        std::sin(targetOrientationAngle*(M_PI/180.0)),      std::cos(targetOrientationAngle*(M_PI/180.0)) );

  double intersectionSum = 0.0;
  for (auto& particle : particles) {
    
    // Build the particle vectors
    double lengthScale = 100.0; 
    TVector2 startingPoint(std::get<0>(particle), std::get<1>(particle));
    TVector2 trajectoryDisplacement( std::sin(std::get<2>(particle)), std::cos(std::get<2>(particle)) );
    trajectoryDisplacement *= lengthScale;

    if (isIntersecting(startingPoint, trajectoryDisplacement, sensitiveStartingPoint, sensitiveTrajectory)) {
      intersectionSum += std::get<3>(particle);
    }

  }

  std::cout << "Plenoptic 1D sum = " << intersectionSum << std::endl;

  // Visualize the plenoptic usage on a 2d histogram (easiest to setup).
  TCanvas canvas(testName.c_str(), testName.c_str(), 700, 700);
  TH2D ranger(testName.c_str(), testName.c_str(), 30, -10.0, 10.0, 30, -10.0, 10.0);
  ranger.Draw();
  
  std::vector<TLine*> lines;

  // Draw scene geometry with lines
  TLine sceneLine;
  sceneLine.SetLineWidth(3);
  for (unsigned int s=0; s<surfaceGeometry.size()-1; s++) {
    TLine* line = sceneLine.DrawLine( std::get<0>(surfaceGeometry[s]),   std::get<1>(surfaceGeometry[s]),
				      std::get<0>(surfaceGeometry[s+1]), std::get<1>(surfaceGeometry[s+1]) );
    lines.push_back(line);
  }

  // Draw some of the generated particles
  unsigned int particlesToDraw = 500;
  unsigned int particlesDrawn = 0;
  TLine particleLine;
  particleLine.SetLineColorAlpha(kBlue-3, 0.2);
  for (auto& particle : particles) {

    if (particlesDrawn > particlesToDraw){
      break;
    }

    double lengthScale = 10.0;

    TLine* line = particleLine.DrawLine( std::get<0>(particle),   
					 std::get<1>(particle),
					 std::get<0>(particle) + lengthScale*std::sin(std::get<2>(particle)), 
					 std::get<1>(particle) + lengthScale*std::cos(std::get<2>(particle)) );
    lines.push_back(line);
    
    particlesDrawn++;
  }

  // Draw the hits
  unsigned int hitsToDraw = 500;
  unsigned int hitsDrawn = 0;
  TLine hitLine;
  hitLine.SetLineColorAlpha(kRed-3, 0.1);
  for (auto& particle : particles) {

    if (hitsDrawn > hitsToDraw){
      break;
    }

    double lengthScale = 10.0;

    TVector2 startingPoint(std::get<0>(particle), std::get<1>(particle));
    TVector2 trajectoryDisplacement(std::sin(std::get<2>(particle)), std::cos(std::get<2>(particle)));
    trajectoryDisplacement *= lengthScale;

    if (isIntersecting(startingPoint, trajectoryDisplacement, sensitiveStartingPoint, sensitiveTrajectory)) {
      TLine* line = hitLine.DrawLine( std::get<0>(particle),   
				      std::get<1>(particle),
				      std::get<0>(particle) + lengthScale*std::sin(std::get<2>(particle)), 
				      std::get<1>(particle) + lengthScale*std::cos(std::get<2>(particle)) );
      lines.push_back(line);
      
      hitsDrawn++; 
    }
  }

  // Draw the target
  lines.push_back( sceneLine.DrawLine( sensitiveStartingPoint.X(), 
				       sensitiveStartingPoint.Y(),
				       sensitiveStartingPoint.X() + sensitiveTrajectory.X(), 
				       sensitiveStartingPoint.Y() + sensitiveTrajectory.Y() )  );
  

  canvas.Update();
  canvas.Write();

}


/*! \brief Generate a simple 2d lightfield under different
 *         parameterisations and check for consistency.
 *
 */
int main(int /*argc*/, char** /*argv*/){

  double targetOrientationAngle = 80.0;
  double sourceMinimumAngle = 0.0;
  double sourceMaximumAngle = 360.0;
  int photonNumberPerSample = 50000;

  // Prepare a root file to store the various results
  TFile testOutput("lightfieldResults.root", "RECREATE");

  // Test plenoptic function in a number of different cases
  testPlenopticFunction("plenoptic1", targetOrientationAngle, sourceMinimumAngle, sourceMaximumAngle, photonNumberPerSample);
  testPlenopticFunction("plenoptic2", 17.0, 20.0, 60.0, photonNumberPerSample);
  testPlenopticFunction("plenoptic3", 100.0, 87.0, 96.0, photonNumberPerSample);
  testPlenopticFunction("plenoptic4", 230.0, 100.0, 180.0, photonNumberPerSample);
  testPlenopticFunction("plenoptic5", 230.0, 150.0, 360.0, photonNumberPerSample);
  testPlenopticFunction("plenoptic6", 230.0, 280.0, 340.0, photonNumberPerSample);
  testPlenopticFunction("plenoptic7", 230.0, 300.0, 320.0, photonNumberPerSample);

  // Save the file and exit
  testOutput.Close();
  return 0;
}
