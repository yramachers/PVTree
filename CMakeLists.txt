cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(PVTree VERSION 0.4.2)
enable_language(Fortran)

# - Allow direct inclusion of our own modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/Modules/")

# - General CMake settings
# Force project directories to appear first in any list of include paths.
set(CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE ON)

# Do not relink a target to any shared library dependencies when
# only the shared library implementation has changed.
set(CMAKE_LINK_DEPENDS_NO_SHARED ON)

# Want relocatability, so always require relative install destinations
set(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION ON)

# Only report new or updated files installed by the ``install`` target.
set(CMAKE_INSTALL_MESSAGE LAZY)

# - Compiler/Language settings
# Always C++11 for now
# NB: Assumes all deps have been compiled against same standard,
#     but we do/can not check this
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Always C11
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_EXTENSIONS OFF)

# Always high warning levels, warings as errors is optional but on by
# default
option(PVTREE_ERROR_ON_WARNINGS "Turn compiler warnings into errors" ON)
foreach(_lang "C" "CXX")
  # GNU/Clang/Intel are mostly common...
  if(CMAKE_${_lang}_COMPILER_ID MATCHES "GNU|(Apple)+Clang|Intel")
    set(PROJECT_${_lang}_FLAGS "-Wall -Wextra -Wshadow -pedantic")

    if(PVTREE_ERROR_ON_WARNINGS)
      set(PROJECT_${_lang}_FLAGS "${PROJECT_${_lang}_FLAGS} -Werror -Wfatal-errors")
    endif()
  endif()

  # Append any user flags
  set(CMAKE_${_lang}_FLAGS "${PROJECT_${_lang}_FLAGS} ${CMAKE_${_lang}_FLAGS}")
endforeach()

# Fortran...
set(PROJECT_Fortran_FLAGS "-Wall -Wno-uninitialized -Wno-conversion")
if(PVTREE_ERROR_ON_WARNINGS)
  set(PROJECT_Fortran_FLAGS "${PROJECT_Fortran_FLAGS} -Werror -Wfatal-errors")
endif()
set(CMAKE_Fortran_FLAGS "${PROJECT_Fortran_FLAGS} ${CMAKE_Fortran_FLAGS}")

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
add_subdirectory(share)

