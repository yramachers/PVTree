#include "pvtree/test/catch.hpp"
#include <libconfig.h++>
#include <iostream>
#include <vector>

TEST_CASE( "libconfig", "[config]" ) {

  std::string fileName = "validation.cfg";
  libconfig::Config cfg;

  // Not a local file so look in the installed share directory
  const char* environmentVariableContents = std::getenv("PVTREE_SHARE_PATH");

  REQUIRE(environmentVariableContents != 0);

  std::string shareFilePath = std::string(environmentVariableContents) + "/config/" + fileName;

  // Try to open and read the configuration file
  bool fileOpened = false;
  try{
    cfg.readFile(shareFilePath.c_str());
    fileOpened = true;
  } catch(const libconfig::FileIOException &fioex){
    std::cerr << "I/O error while reading file." << std::endl;
  }
  catch(const libconfig::ParseException &pex) {
    std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
	      << " - " << pex.getError() << std::endl;
  }

  CHECK(fileOpened);

  // Check file version number
  try {
    std::string version = cfg.lookup("version");
    REQUIRE(version == "1.0");
  } catch(const libconfig::SettingNotFoundException &nfex){
    std::cerr << "No 'version' setting in configuration file." << std::endl;
  }

  // Check window title
  try {
    std::string windowTitle = cfg.lookup("application.window.title");
    REQUIRE(windowTitle == "My Application");
  } catch(const libconfig::SettingNotFoundException &nfex){
    std::cerr << "No 'window title' setting in configuration file." << std::endl;
  }

  // Check value extraction of various types
  try {
    const auto& root = cfg.getRoot();

    double piValue = 0.0;
    root["application"]["misc"].lookupValue("pi", piValue);
    REQUIRE( piValue == 3.141592654 );

    long long bigint = 0;
    root["application"]["misc"].lookupValue("bigint", bigint);
    REQUIRE( bigint == 9223372036854775807L );

    int columnNumber = root["application"]["misc"]["columns"].getLength();
    REQUIRE( columnNumber == 3 );

    std::vector<std::string> columnValues = {"Last Name", "First Name", "MI"};
    for (int c=0; c<columnNumber; c++) {
      std::string element = root["application"]["misc"]["columns"][c];

      REQUIRE( element == columnValues[c] );
    }

    int bitmask = 0;
    root["application"]["misc"].lookupValue("bitmask", bitmask);
    REQUIRE( bitmask == 0x1FC3 );

  } catch(const libconfig::SettingNotFoundException &nfex){
    std::cerr << "No 'window title' setting in configuration file." << std::endl;
  }

  // Check the type deterimination works
  try {
    libconfig::Setting& typeTest = cfg.lookup("application.typeTest");

    REQUIRE(typeTest["test1"].getType() == libconfig::Setting::TypeInt );

    REQUIRE(typeTest["test2"].getType() == libconfig::Setting::TypeFloat );

    REQUIRE(typeTest["test3"].getType() == libconfig::Setting::TypeBoolean );

    REQUIRE(typeTest["test4"].getType() == libconfig::Setting::TypeFloat );

    REQUIRE(typeTest["test5"].getType() == libconfig::Setting::TypeInt64 );

  } catch(const libconfig::SettingNotFoundException &nfex) {
    std::cerr << "Type test has a problem." << std::endl;
  }
}
