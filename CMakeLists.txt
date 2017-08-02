cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(PVTree VERSION 0.4.2)
enable_language(Fortran)

# - Allow direct inclusion of our own modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/Modules/")

# - Always use C++11. NB: does/can not check dependencies are compiled
#   against same standard/ABI this is left as a responsibility of
#   the builder
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#Enable compiler warnings in general
#if(CMAKE_COMPILER_IS_GNUCXX)
  # This is tested
  #  set(CMAKE_CXX_FLAGS     "${CMAKE_CXX_FLAGS} -Wall")
  #set(CMAKE_C_FLAGS       "${CMAKE_C_FLAGS} -Wall -W")
  #set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -Wall -Wno-uninitialized -Wno-conversion " )
  #endif()

# Use a default set of flags
# O2 optimization level may break things in strange ways
#if(NOT(DEFINED CMAKE_BUILD_TYPE) OR (CMAKE_BUILD_TYPE STREQUAL ""))
#  set( CMAKE_CXX_FLAGS     "${CMAKE_CXX_FLAGS} -O0" )
#  set( CMAKE_C_FLAGS       "${CMAKE_C_FLAGS} -O0" )
#  set( CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -O0" )
#endif()


# Use a set of debug flags
# including options for coverage checking.
#if((CMAKE_BUILD_TYPE MATCHES DEBUG) OR (CMAKE_BUILD_TYPE MATCHES COVERAGE) )
  #  message(STATUS "Setting Debug/Coverage flags")
  #  include(CodeCoverage)
  #  set( CMAKE_CXX_FLAGS        "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -fsanitize=undefined -fsanitize=address" )
  #  set( CMAKE_C_FLAGS          "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage -fsanitize=undefined -fsanitize=address" )
  #  set( CMAKE_Fortran_FLAGS    "${CMAKE_Fortran_FLAGS} -fprofile-arcs -ftest-coverage -fsanitize=undefined -fsanitize=address" )
  #  set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage -fsanitize=undefined -fsanitize=address" )

  #  SETUP_TARGET_FOR_COVERAGE(
  #    test_coverage  # Name for custom target.
  #    src/testing/unittest # Name of the test driver executable that runs the tests.
    # NOTE! This should always have a ZERO as exit code
    # otherwise the coverage generation will not complete.
    #    coverageResults  # Name of output directory.
    #    )

  # Make coverage test depend upon unittest being up to date
  #  add_dependencies(test_coverage unittest)
  #endif()

# - Standard POSIX/GNU style install, except for libraries
include(GNUInstallDirs)
set(CMAKE_INSTALL_LIBDIR "${CMAKE_INSTALL_LIBDIR}/${PROJECT_NAME}")
if(NOT IS_ABSOLUTE "${CMAKE_INSTALL_LIBDIR}")
  set(CMAKE_INSTALL_FULL_LIBDIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
else()
  set(CMAKE_INSTALL_FULL_LIBDIR "${CMAKE_INSTALL_LIBDIR}")
endif()

# - Use install dirs to make relocatable build structure
set(PVTREE_BUILDPRODUCTS_DIR ${PROJECT_BINARY_DIR}/BuildProducts)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PVTREE_BUILDPRODUCTS_DIR}/${CMAKE_INSTALL_BINDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PVTREE_BUILDPRODUCTS_DIR}/${CMAKE_INSTALL_LIBDIR})

# Config files have same structure in all trees, so simply softlink
add_custom_command(
  OUTPUT ${PVTREE_BUILDPRODUCTS_DIR}/${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}
  COMMAND ${CMAKE_COMMAND}
          -E make_directory
          ${PVTREE_BUILDPRODUCTS_DIR}/${CMAKE_INSTALL_DATADIR}
  COMMAND ${CMAKE_COMMAND}
          -E create_symlink
          ${PROJECT_SOURCE_DIR}/share
          ${PVTREE_BUILDPRODUCTS_DIR}/${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}
  DEPENDS pvtree-utils
  )
add_custom_target(pvtree-build-data ALL
  DEPENDS ${PVTREE_BUILDPRODUCTS_DIR}/${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}
  )



#-----------------------------------------------------------------------
# - Find Required External Packages
# CERN ROOT for I/O, Math, Analysis
find_package(ROOT REQUIRED MathMore)
include(${ROOT_DIR}/modules/RootNewMacros.cmake)

# Geant4 for particle transport simulation
find_package(Geant4 REQUIRED ui_all vis_all)

# LibConfig for input steering files
# LibConfig's cmake support files are broken, so
# should always trey and use our own Find module
find_package(Libconfig REQUIRED)

# ecCodes for climate data file reading
# eccodes provides a suitable CMake config mode file, but needs
# wrapping to create convenient import targets
find_package(eccodes REQUIRED)
set_property(TARGET eccodes
  APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${ECCODES_INCLUDE_DIRS}"
  )

# - Find Optional External Packages
#find_package(CPPCheck)
#include(CppcheckTargets)
function(add_cppcheck)
endfunction()

# - Set up testing
include(CTest)

# - Build component libraries
add_subdirectory(pvtree)

# - Build user applications
add_subdirectory(programs)

# - Build docs
add_subdirectory(doc)

# - Build standard config files
add_subdirectory(share)

