cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project("PVTree Simulator Superbuild")

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
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif ( CMAKE_COMPILER_IS_GNUCXX )
if ( MSVC )
    # This is untested
    set_property( GLOBAL APPEND_STRING PROPERTY CMAKE_CXX_FLAGS /W3 )
endif ( MSVC )

#Choose an optimization level (if using default build type)
if ( NOT(DEFINED CMAKE_BUILD_TYPE) OR (CMAKE_BUILD_TYPE STREQUAL "") )
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif ( NOT(DEFINED CMAKE_BUILD_TYPE) OR (CMAKE_BUILD_TYPE STREQUAL "") )

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

#Check that user defined paths have been set
if ( NOT(DEFINED PVTREE_SOURCE_DIRECTORY) )
  message(FATAL_ERROR "PVTREE_SOURCE_DIRECTORY not defined by user")
endif ( NOT(DEFINED PVTREE_SOURCE_DIRECTORY) )

if ( NOT(DEFINED PVTREE_CLIMATE_DATA_PATH) )
  message(FATAL_ERROR "PVTREE_CLIMATE_DATA_PATH not defined by user")
endif ( NOT(DEFINED PVTREE_CLIMATE_DATA_PATH) )

IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  message(FATAL_ERROR "CMAKE_INSTALL_PREFIX not defined by user")
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

#In case of external projects not existing.
include(ExternalProject)

#Attempt to download and install ROOT
ExternalProject_Add(SuperROOT
  URL https://root.cern.ch/download/root_v6.02.05.source.tar.gz
  URL_HASH MD5=75d1f2c07960c599ababb6249ad57b4e
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/ROOT-v6-02-05
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}/external/ROOT-v6-02-05
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
  -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
  -Dcxx11=ON
  -Dmathmore=ON
  )


#Check for QT availability
Find_Package(Qt4)

#Attempt to download and install Geant4
#  -DGEANT4_USE_USOLIDS=ON
If(Qt4_FOUND)
  ExternalProject_Add(SuperGeant
    URL http://geant4.cern.ch/support/source/geant4.10.01.p02.tar.gz
    URL_HASH MD5=6a7c088f684262d9f792dcaaf1cd55d1
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/GEANT-v4-10-01-p02
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}/external/GEANT-v4-10-01-p02 
    -DGEANT4_INSTALL_DATA=ON
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DGEANT4_USE_OPENGL_X11=ON
    -DGEANT4_USE_QT=ON
    -DGEANT4_BUILD_CXXSTD=c++11
    )
Else(Qt4_FOUND)
  ExternalProject_Add(SuperGeant
    URL http://geant4.cern.ch/support/source/geant4.10.01.p02.tar.gz
    URL_HASH MD5=6a7c088f684262d9f792dcaaf1cd55d1
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/GEANT-v4-10-01-p02
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}/external/GEANT-v4-10-01-p02
    -DGEANT4_INSTALL_DATA=ON
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DGEANT4_USE_OPENGL_X11=ON
    -DGEANT4_BUILD_CXXSTD=c++11
    )
Endif(Qt4_FOUND)

#Attempt to download and install libconfig
ExternalProject_Add(SuperLibConfig
  URL http://www.hyperrealm.com/libconfig/libconfig-1.5.tar.gz
  URL_HASH MD5=a939c4990d74e6fc1ee62be05716f633
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/LIBCONFIG-v1-05-00
  BUILD_IN_SOURCE 1
  CONFIGURE_COMMAND <SOURCE_DIR>/configure CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER} --prefix=${CMAKE_INSTALL_PREFIX}/external/LIBCONFIG-v1-05-00
  BUILD_COMMAND make
  INSTALL_COMMAND make install
)

# Attempt to install Evolutionary objects library
ExternalProject_Add(SuperEO
  SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/EO-1.3.1/src/SuperEO/eo
  DOWNLOAD_COMMAND /usr/bin/wget -q http://downloads.sourceforge.net/project/eodev/eo/1.3.1/EO-1.3.1.zip
  && unzip -q EO-1.3.1.zip -d <SOURCE_DIR>/..
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/EO-1.3.1
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}/external/EO-1.3.1
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
  -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
  -DENABLE_CMAKE_TESTING=1 
  )

# Attempt to install ecCodes for climate data access
ExternalProject_Add(SuperECCodes
  URL https://software.ecmwf.int/wiki/download/attachments/45757960/eccodes-0.11.0-Source.tar.gz
  URL_HASH MD5=7350ceb7b0bfe478a8e9983c46b314a5
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/ECCODES-v0-11-00
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}/external/ECCODES-v0-11-00
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
)

# Attempt to install cppcheck for static analysis
# See https://github.com/danmar/cppcheck/releases/ for latest versions
ExternalProject_Add(SuperCPPCheck
  URL https://github.com/danmar/cppcheck/archive/1.69.tar.gz
  URL_HASH MD5=9bc7532d9371f506bc4547a05a1fdebf
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/CPPCHECK-01-69
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}/external/CPPCHECK-01-69
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
  -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
  -DBUILD_SHARED_LIBS=OFF
)

#Attempt to install the PVTree source path 
ExternalProject_Add(SuperPVTree
  DEPENDS SuperROOT SuperGeant SuperLibConfig SuperEO SuperECCodes SuperCPPCheck
  SOURCE_DIR ${PVTREE_SOURCE_DIRECTORY} 
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/PVTree
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}/
  -DGeant4_DIR=${CMAKE_INSTALL_PREFIX}/external/GEANT-v4-10-01-p02/lib64/Geant4-10.1.2/
  -DROOT_DIR=${CMAKE_INSTALL_PREFIX}/external/ROOT-v6-02-05/
  -DLIBCONFIG_DIR=${CMAKE_INSTALL_PREFIX}/external/LIBCONFIG-v1-05-00/
  -DEvolutionaryObjects_DIR=${CMAKE_INSTALL_PREFIX}/external/EO-1.3.1/
  -DECCODES_DIR=${CMAKE_INSTALL_PREFIX}/external/ECCODES-v0-11-00/
  -DCLIMATE_DATA_PATH=${PVTREE_CLIMATE_DATA_PATH} 
  -DCPPCheck_DIR=${CMAKE_INSTALL_PREFIX}/external/CPPCHECK-01-69
  )


set(CONFIGURED_ONCE TRUE CACHE INTERNAL
    "A flag showing that CMake has configured at least once.")
