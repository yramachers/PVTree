/*! @file
 * \brief Testing of configuration via file.
 *
 */

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <libconfig.h++>

bool openConfigurationFile(std::string fileName, libconfig::Config& cfg) {

  // Read the file. If there is an error, report it and exit.
  try{
    cfg.readFile(fileName.c_str());
  } catch(const libconfig::FileIOException &fioex){
    std::cerr << "I/O error while reading file." << std::endl;
    return false;
  }
  catch(const libconfig::ParseException &pex) {
    std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
	      << " - " << pex.getError() << std::endl;
    return false;
  }

  return true;
}


int main() {
  libconfig::Config cfg;

  std::string fileName = "validation.cfg";

  // Need to start by finding the file
  // First try to find it w.r.t the local directory
  std::ifstream localTest(fileName.c_str());

  if ( localTest.is_open() ) {
    bool isFileOpen = openConfigurationFile(fileName, cfg);

    if (!isFileOpen) return(EXIT_FAILURE);

  } else {

    // Not a local file so look in the installed share directory
    const char* environmentVariableContents = std::getenv("PVTREE_SHARE_PATH");

    if (environmentVariableContents != 0 ){
      std::string shareFilePath(std::string(environmentVariableContents) + "/config/" + fileName);
      std::ifstream shareTest(shareFilePath.c_str());

      if (shareTest.is_open()) {
	bool isFileOpen = openConfigurationFile(shareFilePath, cfg);

	if (!isFileOpen) return(EXIT_FAILURE);
      } else {

	// Not in either place so give up!
	std::cerr << "Unable to locate file " << fileName << " locally or in the shared config." 
		  << std::endl;
	return(EXIT_FAILURE);
      }
    } else {

      // Not in either place so give up!
      std::cerr << "Unable to locate file " << fileName << " locally or in the shared config." 
		<< std::endl;
      return(EXIT_FAILURE);
    }

  }

  // Get the file version
  try {
    std::string version = cfg.lookup("version");
    std::cout << "Version : " << version << std::endl << std::endl;
  } catch(const libconfig::SettingNotFoundException &nfex){
    std::cerr << "No 'version' setting in configuration file." << std::endl;
  }

  // What about getting a bit more deep
  try {
    std::string windowTitle = cfg.lookup("application.window.title");
    std::cout << "Title : " << windowTitle << std::endl;
  } catch(const libconfig::SettingNotFoundException &nfex){
    std::cerr << "No 'window title' setting in configuration file." << std::endl;
  }

  // What about some misc values
  try {
    const auto& root = cfg.getRoot();
    
    double piValue = 0.0;
    root["application"]["misc"].lookupValue("pi", piValue);
    std::cout << "pi : " << piValue << std::endl;

    long long bigint = 0;
    root["application"]["misc"].lookupValue("bigint", bigint);
    std::cout << "big integer : " << bigint << std::endl;
    
    std::string firstElement = root["application"]["misc"]["columns"][0];
    std::cout << "First column name : " << firstElement << std::endl;
    std::cout << "Number of columns : " << root["application"]["misc"]["columns"].getLength() << std::endl;

  } catch(const libconfig::SettingNotFoundException &nfex){
    std::cerr << "No 'window title' setting in configuration file." << std::endl;
  }

  // Check return types can be determined accurately
  try {
    libconfig::Setting& typeTest = cfg.lookup("application.typeTest");

    if (typeTest["test1"].getType() == libconfig::Setting::TypeInt ) {
      std::cout << "test1 is integer" <<std::endl;
    }

    if (typeTest["test2"].getType() == libconfig::Setting::TypeFloat ) {
      std::cout << "test2 is float" <<std::endl;
    }

    if (typeTest["test3"].getType() == libconfig::Setting::TypeBoolean ) {
      std::cout << "test3 is boolean" <<std::endl;
    }

    if (typeTest["test4"].getType() == libconfig::Setting::TypeFloat ) {
      std::cout << "test4 is float" <<std::endl;
    }

    if (typeTest["test5"].getType() == libconfig::Setting::TypeInt64 ) {
      std::cout << "test5 is long" <<std::endl;
    }
    
  } catch(const libconfig::SettingNotFoundException &nfex) {
    std::cerr << "Type test has a problem." << std::endl;
  }

  return(EXIT_SUCCESS);
}

