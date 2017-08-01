#include "pvtree/utils/resource.hpp"
#define ENABLE_BINRELOC
#include "pvtree_binreloc.h"

namespace {
//! Convert BrInitError code to a string describing the error
//! \todo add errno to returned string
std::string BRErrorAsString(const BrInitError& err) {
  std::string errMsg;
  switch (err) {
    case BR_INIT_ERROR_NOMEM:
      errMsg = "Unable to open /proc/self/maps";
      break;
    case BR_INIT_ERROR_OPEN_MAPS:
      errMsg =  "Unable to read from /proc/self/maps";
      break;
    case BR_INIT_ERROR_READ_MAPS:
      errMsg = "The file format of /proc/self/maps";
      break;
    case BR_INIT_ERROR_INVALID_MAPS:
      errMsg = "The file format of /proc/self/maps is invalid";
      break;
    case BR_INIT_ERROR_DISABLED:
      errMsg = "Binary relocation disabled";
      break;
    default:
      throw std::runtime_error("Invalid BrInitError");
  }
  return errMsg;
}

//! Return relative path from libdir directory to root of resource dir
std::string relativePathToResourceDir() {
    return "@PVTREE_LIBDIR_TO_SHAREDIR@";
}

//! return directory of this library
std::string getLibraryDir() {
  char* exePath(0);
  exePath = br_find_exe_dir("");
  std::string sExePath(exePath);
  free(exePath);
  return sExePath;
}

//! initialize binreloc
bool init_binreloc() {
  static bool isInitialized = false;
  if (!isInitialized) {
    BrInitError err;
    int initSuccessful = br_init_lib(&err);
    if (initSuccessful != 1) {
      throw std::runtime_error("Resource initialization failed: "+BRErrorAsString(err));
    } else {
      isInitialized = true;
  }

  return isInitialized;
}

namespace pvtree {
std::string getConfigFile(const std::string& /*shortpath*/) {
  //Uses PVTREE_SHARE_PATH
  return std::string {};
}

std::string getClimateDataFile(const std::string& /*shortpath*/) {
  // Use PVTREE_CLIMATE_DATAPATH
  return std::string {};
}

} // namespace pvtree

