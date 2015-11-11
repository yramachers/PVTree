#include "solarSimulation/spectrum.hpp"
#include "utils/equality.hpp"

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <stdexcept>
#include <sstream>
#include <cmath>
#include <algorithm>

#include "TH1D.h"
#include "TFile.h"

#include "CLHEP/Units/SystemOfUnits.h"
#include "CLHEP/Units/PhysicalConstants.h"

Spectrum::Spectrum(std::string inputFilePath) : m_dataPrecision(10000) {

  // Need to start by finding the file
  // First try to find it w.r.t the local directory
  std::ifstream localTest(inputFilePath.c_str());

  if ( localTest.is_open() ) {
    // If found use the local file
    extractFile( localTest ); 
    return;
  }

  // Not a local file so look in the installed share directory
  const char* environmentVariableContents = std::getenv("PVTREE_SHARE_PATH");
  
  if ( environmentVariableContents != 0 ) {

    //Environment variable set so give it a try
    std::string   shareFilePath(std::string(environmentVariableContents) + "/" + inputFilePath);
    std::ifstream shareTest(shareFilePath.c_str());
      
    if ( shareTest.is_open() ) {
      extractFile( shareTest );
      return;
    }
  }

  // If reaching here then unable to extract a file's contents!
  std::cout << "Spectrum::Spectrum - Unable to find the specified input file " << inputFilePath 
	    << std::endl;
  throw std::invalid_argument("Can't find spectrum input file.");
}

Spectrum::Spectrum(std::vector<std::string> columnNames,
		   std::map<std::string, std::vector<double> > data) 
  : m_data(data), m_columnNames(columnNames), m_dataPrecision(10) {
  
}

Spectrum::~Spectrum() {}

std::vector<std::tuple<double, double> > Spectrum::generatePhotons(unsigned int photonNumber) {

  auto normalIrradianceHistogram = getHistogram("Direct_normal_irradiance");

  std::vector<std::tuple<double, double>> generatedPhotons;

  // Need to include width of bin!
  double totalIrradianceSum = normalIrradianceHistogram->Integral("width");

  for (unsigned int r=0; r< photonNumber; r++ ) {
    double wavelength = normalIrradianceHistogram->GetRandom(); //nm

    //Convert the wavelength (nm) into energy (eV)
    double energy = CLHEP::h_Planck * CLHEP::c_light / (wavelength * CLHEP::nm); //Joules
    energy /= CLHEP::eV; //to eV 

    generatedPhotons.push_back(std::make_tuple(energy, totalIrradianceSum) );
  }

  return generatedPhotons;
}

std::vector<std::string> Spectrum::getSMARTSColumnNames() const {
  return m_columnNames;
}

std::map<std::string, std::vector<double> > Spectrum::getSMARTSData() const {
  return m_data;
}

bool Spectrum::operator==(const Spectrum& otherSpectrum) {

  // Check that the column names are the same
  if (this->getSMARTSColumnNames().size() != 
      otherSpectrum.getSMARTSColumnNames().size()) {
    return false;
  }

  // Perhaps allow for variation in ordering of column names as well?
  if( !  std::equal(this->m_columnNames.begin(), this->m_columnNames.end(), 
		    otherSpectrum.m_columnNames.begin()) ){
    return false;
  }

  // Get the largest data precision check as SMARTS stores < float precision
  // in the export file.
  int largestPrecision = this->m_dataPrecision > otherSpectrum.m_dataPrecision ? 
    this->m_dataPrecision : otherSpectrum.m_dataPrecision;
  
  // Check that the data values per column are the same
  for (auto& name : this->getSMARTSColumnNames()) {
    std::vector<double > data1 = this->getSMARTSData()[name];
    std::vector<double > data2 = otherSpectrum.getSMARTSData()[name];
    
    if (data1.size() != data2.size()) {
      return false;
    }

    for (unsigned int x=0; x<data1.size(); x++) {
      if (!almost_equal(float(data1[x]), float(data2[x]), largestPrecision)) {
	return false;
      }
    }
  }

  // If reaching this point they can be considered identical
  return true; 
}

bool Spectrum::operator!=(const Spectrum& otherSpectrum) {
  return !(*this == otherSpectrum);
}

void Spectrum::extractFile(std::ifstream& inputFile) {

  char line[512];

  bool extractingHeader = true;

  while ( inputFile.getline(line, 512) ){
    std::string currentLine(line);
    std::stringstream currentLineStream(currentLine);

    if (extractingHeader) {
      // Assume first line contains table header names
      std::string columnName;
      while ( currentLineStream >> columnName ) {
	m_columnNames.push_back(columnName);
      }

      // Create vector<double>s to store results
      for ( auto& column : m_columnNames ){
	m_data[column] = std::vector<double>();
      }

      extractingHeader = false;
      continue;
    }

    // Keep reading until end of file to get the row values!
    unsigned int columnIndex =0;
    std::string rowValue;
    while (currentLineStream >> rowValue) {
      if ( columnIndex >= m_columnNames.size() ) {
	std::cout << "Current line has too many values: - \n" << currentLine << std::endl;
	throw std::exception();
      }

      m_data[m_columnNames[columnIndex]].push_back( std::stod(rowValue) );
      columnIndex++;
    }

  }
}

void Spectrum::createHistogram(std::string columnName) {

  // Get the binning
  std::vector<double> binWidths;
  std::vector<double> binValues;
  std::vector<double> binLowEdges;

  // Fill the first bin width as a special case
  binWidths.push_back( m_data[std::string("Wvlgth")][1] - m_data[std::string("Wvlgth")][0]  );
  binValues.push_back( m_data[columnName][0] );
  binLowEdges.push_back( m_data[std::string("Wvlgth")][0] - binWidths[0]/2.0 );

  for (unsigned int b=1; b<m_data[std::string("Wvlgth")].size(); b++) {
    
    double distanceToPreviousBinCentre = m_data[std::string("Wvlgth")][b] - m_data[std::string("Wvlgth")][b-1];

    // Use the previous bin size to get the next bin size.
    double currentBinHalfWidth = distanceToPreviousBinCentre - (binWidths.back()/2.0);

    if (currentBinHalfWidth < 0.0){
      std::cerr << "Can't use negative bin widths. Logic problem in irradiance histogram creation!" << std::endl;
      throw;
    }

    binWidths.push_back( 2.0*currentBinHalfWidth );
    binValues.push_back( m_data[columnName][b] );
    binLowEdges.push_back( m_data[std::string("Wvlgth")][b] - binWidths[b]/2.0 );
  }

  // Add one additional low bin edge (and zero value)
  binValues.push_back( 0.0 );
  binLowEdges.push_back( m_data[std::string("Wvlgth")].back() + binWidths.back()/2.0 );

  // Build an array of low edges
  double* binLowEdgeArray = new double[binLowEdges.size()];
  std::copy(binLowEdges.begin(), binLowEdges.end(), binLowEdgeArray);

  // Fill a histogram for the photon flux as a function of photon wavelength
  auto histogram = std::shared_ptr<TH1D>(new TH1D(columnName.c_str(), 
						  columnName.c_str(), 
						  binLowEdges.size()-1, 
						  binLowEdgeArray));

  // I don't want ROOT to manage the memory of this object.
  histogram->SetDirectory(0); 

  // Set all the bin values
  for (unsigned int b=0; b<binValues.size(); b++) {
    histogram->SetBinContent(b+1, binValues[b]);
  }

  // Add the histogram to the map of previously created histograms
  m_histograms[columnName] = histogram;

  // Delete the low edges
  delete[] binLowEdgeArray;
}

std::shared_ptr<TH1D> Spectrum::getHistogram(std::string columnName) {

  // Check if already created
  if ( m_histograms.find(columnName) != m_histograms.end() ) {
    return m_histograms[columnName];
  }

  // Check if the column requested was actually produced by SMARTS
  if ( std::find(begin(m_columnNames), end(m_columnNames), columnName) == m_columnNames.end() ) {
    std::cerr << "SMARTS has not produced the column: " << columnName << std::endl;
    std::cerr << "SMARTS has produced the following columns: - " << std::endl;
    for (auto name : m_columnNames){
      std::cerr << "\t" << name << std::endl;
    }

    throw std::string("SMARTS has not produced the column: ") + columnName;
  }

  // Finally try creating it
  createHistogram(columnName);
  return m_histograms[columnName];
}


