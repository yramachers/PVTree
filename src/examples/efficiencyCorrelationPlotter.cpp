/*!
 * @file 
 * \brief Plot the correlations of all the parameters with the
 *        efficiency of the tree structure.
 *
 * Considers all the trees in a TList contained within a file. It
 * is necessary to run of the file twice because the ranges of the
 * parameters/results need to be determined.
 */
#include "treeSystem/treeConstructionInterface.hpp"
#include "leafSystem/leafConstructionInterface.hpp"
#include "analysis/yearlyResult.hpp"
#include "utils/getopt_pp.h"
#include "utils/equality.hpp"
#include <iostream>
#include <string>
#include <utility>
#include <tuple>

// ROOT includes
#include "TFile.h"
#include "TList.h"
#include "TH2D.h"

/*! \brief Generic class to handle evaluation of derived variables and their
 *         eventual plotting.
 */
template<typename T>
class DerivedVariable {
public:
  std::string m_label;
  std::string m_persistencePrefix;
  std::function<T(TreeConstructionInterface*,LeafConstructionInterface*)> m_evaluate;
  T m_minimumValue;
  T m_maximumValue;
  
  DerivedVariable(std::string label, 
		  std::string prefix,
		  std::function<T(TreeConstructionInterface*,LeafConstructionInterface*)> evaluateFunction) : 
    m_label(label), m_persistencePrefix(prefix), m_evaluate(evaluateFunction) {}
};

/*! \brief Generic class to apply selection
 */
class AnalysisSelection {
public:
  std::string m_label;
  std::function<bool(TreeConstructionInterface*,LeafConstructionInterface*)> m_evaluate;
  
  AnalysisSelection(std::string label, 
		  std::function<bool(TreeConstructionInterface*,LeafConstructionInterface*)> evaluateFunction) : 
    m_label(label), m_evaluate(evaluateFunction) {}
};

/*! \brief Extract the tree double parameters
 *
 */
void getTreeParameters(std::vector<DerivedVariable<double>>& plotParameters, TList* structureList) {

  TreeConstructionInterface* currentSystem = ((YearlyResult*)(structureList->At(0))) ->getTree();
  std::vector<std::string> parameterNames = currentSystem->getDoubleParameterNames();

  for (auto& parameterName : parameterNames) {

    plotParameters.push_back(DerivedVariable<double>(parameterName,
						     "tree_" + parameterName,
						     [parameterName](TreeConstructionInterface* t, LeafConstructionInterface* /* l*/) -> double {
						       return t->getDoubleParameter(parameterName);})
			     );
    
  } 

  // Repeat for integer parameters, might be a little squiffy :D
  parameterNames = currentSystem->getIntegerParameterNames();

  for (auto& parameterName : parameterNames) {

    plotParameters.push_back(DerivedVariable<double>(parameterName,
						     "tree_" + parameterName,
						     [parameterName](TreeConstructionInterface* t, LeafConstructionInterface* /* l*/) -> double {
						       return t->getIntegerParameter(parameterName);})
			     );
    
  } 
}

/*! \brief Extract the leaf double parameters
 *
 */
void getLeafParameters(std::vector<DerivedVariable<double>>& plotParameters, TList* structureList) {

  LeafConstructionInterface* currentSystem = ((YearlyResult*)(structureList->At(0))) ->getLeaf();
  std::vector<std::string> parameterNames = currentSystem->getDoubleParameterNames();

  for (auto& parameterName : parameterNames) {

    plotParameters.push_back(DerivedVariable<double>(parameterName,
						     "leaf_" + parameterName,
						     [parameterName](TreeConstructionInterface* /*t*/, LeafConstructionInterface* l) -> double {
						       return l->getDoubleParameter(parameterName);})
			     );
    
  }

  // Repeat for integer parameters, might be a little squiffy :D
  parameterNames = currentSystem->getIntegerParameterNames();

  for (auto& parameterName : parameterNames) {

    plotParameters.push_back(DerivedVariable<double>(parameterName,
						     "leaf_" + parameterName,
						     [parameterName](TreeConstructionInterface* /*t*/, LeafConstructionInterface* l) -> double {
						       return l->getIntegerParameter(parameterName);})
			     );
    
  }
}


/*! \brief Extract double parameter ranges
 *
 */
void getParameterRanges(TList* structureList, 
			std::vector<DerivedVariable<double>>& plotParameters,
			std::vector<AnalysisSelection>& selections) {

  // Handle initialization gracefully
  bool initialRangesInitialized = false;

  // Need an iterator for each list
  TIter structureListIterator(structureList);

  // Identify the value ranges for plotting
  for (int t=0; t<structureList->GetSize(); t++){
    YearlyResult* currentStructure = (YearlyResult*)structureListIterator();
    TreeConstructionInterface* currentTreeSystem = currentStructure->getTree();
    LeafConstructionInterface* currentLeafSystem = currentStructure->getLeaf();

    // Apply selection
    bool keepStructure = true;

    for (auto& selection : selections) {
      if (!selection.m_evaluate(currentTreeSystem, currentLeafSystem)) {
	keepStructure = false;
	break;
      }
    }

    if (!keepStructure) {
      continue;
    }

    for (auto& plotParameter : plotParameters) {
      double currentValue = plotParameter.m_evaluate(currentTreeSystem, currentLeafSystem);

      // Initialize if necessary
      if (!initialRangesInitialized){

	plotParameter.m_minimumValue = currentValue;
	plotParameter.m_maximumValue = currentValue;

      } else {

	if (currentValue < plotParameter.m_minimumValue) {
	  plotParameter.m_minimumValue = currentValue;
	}
	
	if (currentValue > plotParameter.m_maximumValue) {
	  plotParameter.m_maximumValue = currentValue;
	}

      }
    }

    // If reaching here the ranges must have been initialized
    initialRangesInitialized = true;
  } 
}

/*! \brief Create a set of histograms
 *
 */
void fillHistograms(std::string histogramSetPrefix, 
		    TList* structureList,
		    std::vector<AnalysisSelection>& selections,
		    std::vector<DerivedVariable<double>>& plotParameters,
		    std::vector<DerivedVariable<double>>& yAxisParameters) {

  int printEvery = 10000;

  // Hard code some bin numbers
  int xAxisBinNumber = 100;
  int yAxisBinNumber = 100;

  // Are the min and max values separated? 
  int comparisonPrecision = 10;

  // Create histogram to hold the cut flow
  std::string cutFlowName = histogramSetPrefix + "_cutFlow";
  std::string cutFlowTitle = ";Cut;Tree Number";
  TH1D cutFlowHistogram(cutFlowName.c_str(), cutFlowTitle.c_str(), selections.size(), 0, selections.size());
  
  for ( unsigned int s=0; s<selections.size(); s++) {
    cutFlowHistogram.GetXaxis()->SetBinLabel(s+1, selections[s].m_label.c_str() );
  }

  // Create the 2D histograms 
  std::vector<std::tuple<DerivedVariable<double>, DerivedVariable<double>, TH2D*>> histograms;

  for ( auto& xAxisParameter : plotParameters ) {

    int currentXBinNumber = xAxisBinNumber;
    double currentXBinMinimum = xAxisParameter.m_minimumValue;
    double currentXBinMaximum = xAxisParameter.m_maximumValue;
    if (almost_equal(xAxisParameter.m_minimumValue, xAxisParameter.m_maximumValue, comparisonPrecision)) {
      currentXBinNumber = 1;
      currentXBinMinimum = xAxisParameter.m_minimumValue - xAxisParameter.m_minimumValue/1000.0;
      currentXBinMaximum = xAxisParameter.m_maximumValue + xAxisParameter.m_maximumValue/1000.0;
    } 

    for ( auto& yAxisParameter : yAxisParameters ) {
      std::string histogramName = histogramSetPrefix + "_"
	+ yAxisParameter.m_persistencePrefix + "_Vs_" + xAxisParameter.m_persistencePrefix;
      std::string histogramTitle = ";" + xAxisParameter.m_label + ";" + yAxisParameter.m_label;

      int currentYBinNumber = yAxisBinNumber;
      double currentYBinMinimum = yAxisParameter.m_minimumValue;
      double currentYBinMaximum = yAxisParameter.m_maximumValue;
      if (almost_equal(yAxisParameter.m_minimumValue, yAxisParameter.m_maximumValue, comparisonPrecision)) {
	currentYBinNumber = 1;
	currentYBinMinimum = yAxisParameter.m_minimumValue - yAxisParameter.m_minimumValue/1000.0;
	currentYBinMaximum = yAxisParameter.m_maximumValue + yAxisParameter.m_maximumValue/1000.0;
      } 


      TH2D* hist = new TH2D(histogramName.c_str(),
			    histogramTitle.c_str(),
			    currentXBinNumber,
			    currentXBinMinimum,
			    currentXBinMaximum,
			    currentYBinNumber,
			    currentYBinMinimum,
			    currentYBinMaximum);

      histograms.push_back( std::make_tuple(xAxisParameter, yAxisParameter, hist) );

    }
  }

  // Need an iterator for each list
  TIter structureListIterator(structureList);

  for (int t=0; t<structureList->GetSize(); t++) {

    if (t % printEvery  == 0){
      std::cout << "Considering tree-leaf pair " << t << std::endl;
    }

    YearlyResult* currentStructure = (YearlyResult*)structureListIterator();
    TreeConstructionInterface* treeSystem = currentStructure->getTree();
    LeafConstructionInterface* leafSystem = currentStructure->getLeaf();

    // Apply any selection
    bool keepStructure = true;

    for (auto& selection : selections) {
      if (!selection.m_evaluate(treeSystem, leafSystem)) {
	keepStructure = false;
	break;
      } else {
	// If passing add to the cut flow.
	cutFlowHistogram.Fill(selection.m_label.c_str(), 1.0);
      }
    }

    if (!keepStructure) {
      continue;
    }

    for (auto& histogramTuple : histograms) {
      double xValue = std::get<0>(histogramTuple).m_evaluate(treeSystem, leafSystem);
      double yValue = std::get<1>(histogramTuple).m_evaluate(treeSystem, leafSystem);
      std::get<2>(histogramTuple)->Fill(xValue, yValue, 1.0);
    }
  }

  // Write the histograms to the root file
  for ( auto& histogramTuple : histograms ){
    std::get<2>(histogramTuple)->Write();
  }

  cutFlowHistogram.Write();
}

void showHelp() {
  std::cout << "efficiencyCorrelationPlotter help" << std::endl;
  std::cout << "\t -i, --inputRootFile <ROOT FILE NAME>" << std::endl;
  std::cout << "\t -o, --outputRootFile <ROOT FILE NAME>" << std::endl;
}

int main(int argc, char** argv) {
  std::string filename;
  std::string outputFilename;

  GetOpt::GetOpt_pp ops(argc, argv);

  // Check for help request
  if (ops >> GetOpt::OptionPresent('h', "help")){
    showHelp();
    return 0;
  }

  ops >> GetOpt::Option('i', "inputRootFile",  filename,       "");
  ops >> GetOpt::Option('o', "outputRootFile", outputFilename, "efficiencyCorrelation.results.root");

  if (filename == ""){
    std::cerr << "Empty input filename" << std::endl;
    showHelp();
    return -1;
  }

  if (outputFilename == ""){
    std::cerr << "Empty output filename" << std::endl;
    showHelp();
    return -1;
  }

  // Also do not run if other arguments are present
  if (ops.options_remain()) {
    std::cerr << "Oops! Unexpected options." << std::endl;
    showHelp();
    return -1;
  }

  TFile ff(filename.c_str(),"READ");
  TList* structureList = (TList*)ff.Get("testedStructures");

  // Check there is at least one structure present
  if ( structureList->GetSize() == 0 ){
    std::cout << "There are no structures to consider." << std::endl;
    return 1;
  }

  // Get all the plottable parameters
  std::vector<DerivedVariable<double>> plotParameters;

  // Get standard tree parameters
  getTreeParameters(plotParameters, structureList);

  // Get standard leaf parameters
  getLeafParameters(plotParameters, structureList);

  // Create some derived parameters using lambda functions
  plotParameters.push_back(DerivedVariable<double>("Surface Density",
						   "tree_surfaceDensity",
						   [](TreeConstructionInterface* t, LeafConstructionInterface* /* l*/) -> double {
						     return t->getDoubleParameter("totalEnergy")/t->getDoubleParameter("sensitiveArea");})
			   );
  plotParameters.push_back(DerivedVariable<double>("Floor Surface Density",
						   "tree_floorSurfaceDensity",
						   [](TreeConstructionInterface* t, LeafConstructionInterface* /* l*/) -> double {
						     return t->getDoubleParameter("totalEnergy")/( t->getDoubleParameter("structureXSize")*t->getDoubleParameter("structureYSize")  );})
			   );
  plotParameters.push_back(DerivedVariable<double>("Volume Density",
						   "tree_volumeDensity",
						   [](TreeConstructionInterface* t, LeafConstructionInterface* /* l*/) -> double {
						     return t->getDoubleParameter("totalEnergy")/( t->getDoubleParameter("structureXSize")*t->getDoubleParameter("structureYSize")*t->getDoubleParameter("structureZSize")  );})
			   );
  plotParameters.push_back(DerivedVariable<double>("Fractional Energy Error",
						   "tree_fracError",
						   [](TreeConstructionInterface* t, LeafConstructionInterface* /* l*/) -> double {
						     return t->getDoubleParameter("totalEnergyStdDeviation")/t->getDoubleParameter("totalEnergy");})
			   );


  // Create some analysis selection
  std::vector<AnalysisSelection> analysis;

  // For a total count
  analysis.push_back(AnalysisSelection("Exists",
				       [](TreeConstructionInterface* /* t*/, LeafConstructionInterface* /* l*/) -> bool {
					 return true;})
		     );

  // Just require a tree has non-zero surface area
  analysis.push_back(AnalysisSelection("Has Surface Area",
				       [](TreeConstructionInterface* t, LeafConstructionInterface* /* l*/) -> bool {
					 return (t->getDoubleParameter("sensitiveArea") != 0.0) ;})
		     );

  // Obtain parameter ranges
  getParameterRanges(structureList, plotParameters, analysis);

  // Create plots against the derived parameters
  // Open a root file to contain the results
  TFile resultsFile(outputFilename.c_str(),"RECREATE");

  // Select some parameters which should appear on the y-axis
  // Could just do every possible combination, but that might be a bit wasteful
  std::vector<std::string> selectedYAxisParameterNames = { "tree_surfaceDensity", 
							   "tree_floorSurfaceDensity", 
							   "tree_volumeDensity",
							   "tree_leafNumber",
							   "tree_fracError"};

  std::vector<DerivedVariable<double>> selectedYAxisParameters;
  for ( auto& parameterName : selectedYAxisParameterNames ){
    for ( auto& availableParameter : plotParameters ){
      if (parameterName == availableParameter.m_persistencePrefix) {
	selectedYAxisParameters.push_back(availableParameter);
      }
    }
  }

  // Create the histograms
  fillHistograms("", structureList, analysis, plotParameters, selectedYAxisParameters);

  // Tidy up
  resultsFile.Close();
}
