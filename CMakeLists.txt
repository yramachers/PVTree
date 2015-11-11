cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project("PVTree Simulator")
enable_language(Fortran)

# The project version number.
set(VERSION_MAJOR   0   CACHE STRING "Project major version number.")
set(VERSION_MINOR   4   CACHE STRING "Project minor version number.")
set(VERSION_PATCH   2   CACHE STRING "Project patch version number.")
mark_as_advanced(VERSION_MAJOR VERSION_MINOR VERSION_PATCH)

INCLUDE(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" HAS_CXX11)
if(NOT HAS_CXX11)
  message(ERROR "Current compiler does not suppport -std=c++11 option.")
endif(NOT HAS_CXX11)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

if(APPLE)
  # Need to use libc++ on Mac
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

#Enable compiler warnings in general
if ( CMAKE_COMPILER_IS_GNUCXX )
    # This is tested
    set( CMAKE_CXX_FLAGS     "${CMAKE_CXX_FLAGS} -Wall")
    set( CMAKE_C_FLAGS       "${CMAKE_C_FLAGS} -Wall -W")
    set( CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -Wall -Wno-uninitialized -Wno-conversion " )

endif ( CMAKE_COMPILER_IS_GNUCXX )
if ( MSVC )
    # This is untested
    set_property( GLOBAL APPEND_STRING PROPERTY CMAKE_CXX_FLAGS /W3 )
endif ( MSVC )

# Use a default set of flags
# O2 optimization level may break things in strange ways
if ( NOT(DEFINED CMAKE_BUILD_TYPE) OR (CMAKE_BUILD_TYPE STREQUAL "") )
  set( CMAKE_CXX_FLAGS     "${CMAKE_CXX_FLAGS} -O0" )
  set( CMAKE_C_FLAGS       "${CMAKE_C_FLAGS} -O0" )
  set( CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -O0" )

endif ( NOT(DEFINED CMAKE_BUILD_TYPE) OR (CMAKE_BUILD_TYPE STREQUAL "") )

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# Use a set of debug flags
# including options for coverage checking.
if ( (CMAKE_BUILD_TYPE MATCHES DEBUG) OR (CMAKE_BUILD_TYPE MATCHES COVERAGE) )
  message(STATUS "Setting Debug/Coverage flags")
  include(CodeCoverage)
  set( CMAKE_CXX_FLAGS        "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -fsanitize=undefined -fsanitize=address" )
  set( CMAKE_C_FLAGS          "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage -fsanitize=undefined -fsanitize=address" )
  set( CMAKE_Fortran_FLAGS    "${CMAKE_Fortran_FLAGS} -fprofile-arcs -ftest-coverage -fsanitize=undefined -fsanitize=address" )
  set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage -fsanitize=undefined -fsanitize=address" )

  SETUP_TARGET_FOR_COVERAGE(
    test_coverage  # Name for custom target.
    src/testing/unittest # Name of the test driver executable that runs the tests.
    # NOTE! This should always have a ZERO as exit code
    # otherwise the coverage generation will not complete.
    coverageResults  # Name of output directory.
    )

  # Make coverage test depend upon unittest being up to date
  add_dependencies(test_coverage unittest)
endif ( (CMAKE_BUILD_TYPE MATCHES DEBUG) OR (CMAKE_BUILD_TYPE MATCHES COVERAGE) )

#In case of external projects not existing.
include(ExternalProject)

#Before testing for root using standard cmake script setup the environment!
if ( NOT(DEFINED ROOT_DIR) )
  message(FATAL_ERROR "ROOT_DIR not defined. Therefore should not be able to find root.")
endif ( NOT(DEFINED ROOT_DIR) )

#Ensure that root-config can be found
set(ENV{PATH} "$ENV{PATH}:${ROOT_DIR}/bin")

#Require ROOT, initially try finding previously installed root
find_package(ROOT REQUIRED)

if (NOT ROOT_FOUND)
  message(FATAL_ERROR "ROOT NOT found.")
else (NOT ROOT_FOUND)

  set(ROOT_INSTALL_PREFIX ${ROOTSYS})

endif (NOT ROOT_FOUND)

include_directories(BEFORE ${ROOT_INCLUDE_DIR})
set(LIBS ${LIBS} ${ROOT_LIBRARIES})

#Require Geant4, try to find a previously installed version first
if (NOT (DEFINED Geant4_DIR))
  message(FATAL_ERROR "Geant4 NOT found.")
else (NOT (DEFINED Geant4_DIR))

  #Require Geant4
  option(WITH_GEANT4_UIVIS "Build with Geant4 UI and Vis drivers" ON)
  if(WITH_GEANT4_UIVIS)
    find_package(Geant4 REQUIRED ui_all vis_all)
  else(WITH_GEANT4_UIVIS)
    find_package(Geant4 REQUIRED)
  endif(WITH_GEANT4_UIVIS)

  #Setup header path, compiler flags and compiler definitions for linking to 
  #Geant4.
  set(Geant4_LIBRARY_DIR ${Geant4_DIR}/../)
  include(${Geant4_USE_FILE})

  #For pointing at the setup script
  set(Geant4_INSTALL_PREFIX ${Geant4_DIR}/../..)

endif (NOT (DEFINED Geant4_DIR))

include_directories(BEFORE ${Geant4_INCLUDE_DIR})
set(LIBS ${LIBS} ${Geant4_LIBRARIES})

# Require that libconfig is present for file configuration handling.
if ( NOT(DEFINED LIBCONFIG_DIR) )
  message(FATAL_ERROR "LIBCONFIG_DIR not defined. Therefore should not be able to find libconfig++.")
endif ( NOT(DEFINED LIBCONFIG_DIR) )

FIND_LIBRARY(LIBCONFIG_LIBRARY NAMES config++ PATHS ${LIBCONFIG_DIR}/lib /usr/lib usr/local/lib)
FIND_PATH(LIBCONFIG_INCLUDE_DIR libconfig.h++ ${LIBCONFIG_DIR}/include /usr/include /usr/local/include)

include_directories(BEFORE ${LIBCONFIG_INCLUDE_DIR})
set(LIBS ${LIBS} ${LIBCONFIG_LIBRARY})

# For setup script
set(LIBCONFIG_INSTALL_PREFIX ${LIBCONFIG_DIR})

# Require that Evolutionary Objects is present
if (NOT (DEFINED EvolutionaryObjects_DIR))
  message(FATAL_ERROR "Evolutionary Objects library not found.")
else (NOT (DEFINED EvolutionaryObjects_DIR))

 set(EvolutionaryObjects_LIBRARY_DIR ${EvolutionaryObjects_DIR}/lib)
 set(EvolutionaryObjects_INCLUDE_DIR ${EvolutionaryObjects_DIR}/include/eo)
 set(EvolutionaryObjects_LIBS "-L${EvolutionaryObjects_LIBRARY_DIR} -lcma  -leo  -leoserial  -leoutils  -les  -lga")

endif (NOT (DEFINED EvolutionaryObjects_DIR))

#Also include evolutionary computing library
include_directories(BEFORE ${EvolutionaryObjects_INCLUDE_DIR})
set(LIBS ${LIBS} ${EvolutionaryObjects_LIBS})

# Require that the ecCodes library is present for handling climate data
if ( NOT(DEFINED ECCODES_DIR) )
  message(FATAL_ERROR "ECCODES_DIR not defined. Therefore can not interpret climate data for simulation.")
endif ( NOT(DEFINED ECCODES_DIR) )

FIND_LIBRARY(ECCODES_LIBRARY NAMES eccodes PATHS ${ECCODES_DIR}/lib)
FIND_PATH(ECCODES_INCLUDE_DIR eccodes.h ${ECCODES_DIR}/include)

include_directories(BEFORE ${ECCODES_INCLUDE_DIR})
set(LIBS ${LIBS} ${ECCODES_LIBRARY})

# For setup script
set(ECCODES_INSTALL_PREFIX ${ECCODES_DIR})

# Setup static analysis
# Need to include the super build install path first
set(CPPCHECK_SEARCH_PATHS "${CPPCheck_DIR}/")
find_package(CPPCheck REQUIRED)

if (NOT CPPCheck_FOUND)
  message(FATAL_ERROR "CPP Check NOT found.")
endif (NOT CPPCheck_FOUND)

include(CppcheckTargets)

enable_testing(true)

# Add the include directory (just version.hpp creation for now)
add_subdirectory(include)

#Directory containing all of our code to be compiled
add_subdirectory(src)

#Directory containing the documentation generation configuration
add_subdirectory(doc)

#Directory containing scripts to be generated
add_subdirectory(scripts)

#Example batch scripts are also useful
add_subdirectory(batch) 

#Directory containing validation spectra
add_subdirectory(spectra)

#For SMARTS integration need to install additional files.
add_subdirectory(smarts)

# Add directory containing configuration files
add_subdirectory(config)

#Install the include directory, but skip pre-gen/cmake files
install(DIRECTORY include DESTINATION .
  PATTERN *.in EXCLUDE
  PATTERN CMakeLists.txt EXCLUDE)

set(CONFIGURED_ONCE TRUE CACHE INTERNAL
    "A flag showing that CMake has configured at least once.")
