#ifndef RESOURCE_HH
#define RESOURCE_HH

#include <string>

namespace pvtree {
/// Return full path to a configuration resource
std::string getConfigFile(const std::string& shortpath);

/// Return full path to a climate data file
std::string getClimateDataFile(const std::string& shortpath);

} // namespace pvtree

#endif // RESOURCE_HH

