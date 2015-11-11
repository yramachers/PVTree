# - Run cppcheck on c++ source files as a custom target and a test
#
#  include(CppcheckTargets)
#  add_cppcheck(<target-name> [UNUSED_FUNCTIONS] [STYLE] [POSSIBLE_ERROR] [FORCE] [FAIL_ON_WARNINGS]) -
#    Create a target to check a target's sources with cppcheck and the indicated options
#
# Requires these CMake modules:
#  Findcppcheck
#
# Requires CMake 2.6 or newer (uses the 'function' command)
#
# Original Author:
# 2009-2010 Ryan Pavlik <rpavlik@iastate.edu> <abiryan@ryand.net>
# http://academic.cleardefinition.com
# Iowa State University HCI Graduate Program/VRAC
#
# Copyright Iowa State University 2009-2010.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

if(__add_cppcheck)
  return()
endif()
set(__add_cppcheck YES)

if(NOT CPPCheck_FOUND)
  find_package(cppcheck QUIET)
endif()

function(add_cppcheck _name)
  set(options UNUSED_FUNCTIONS FORCE )
  set(oneValueArgs )
  set(multiValueArgs SUPPRESS)
  cmake_parse_arguments(add_cppcheck "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

  if(NOT TARGET ${_name})
    message(FATAL_ERROR
      "add_cppcheck given a target name that does not exist: '${_name}' !")
  endif()
	
  if(CPPCheck_FOUND)
    set(_cppcheck_args)
    
    if(${add_cppcheck_UNUSED_FUNCTIONS})
      list(APPEND _cppcheck_args "--enable=unusedFunction")
    endif(${add_cppcheck_UNUSED_FUNCTIONS})
    
    if(${add_cppcheck_FORCE})
      list(APPEND _cppcheck_args "--force")
    endif()

    foreach(_suppression ${add_cppcheck_SUPPRESS})
      list(APPEND _cppcheck_args "--suppress=${_suppression}")
    endforeach()

    #It would be better to add each include file (and ignore bad ones!)
    set(_includes "-I${CMAKE_SOURCE_DIR}/include/")

    get_target_property(_cppcheck_sources "${_name}" SOURCES)
    set(_files)
    foreach(_source ${_cppcheck_sources})
      get_source_file_property(_cppcheck_lang "${_source}" LANGUAGE)
      get_source_file_property(_cppcheck_loc "${_source}" LOCATION)
      if("${_cppcheck_lang}" MATCHES "CXX" OR "${_cppcheck_lang}" MATCHES "C")
	list(APPEND _files "${_cppcheck_loc}")
      endif()
    endforeach()
    
    # Choose what kind of checks to run by default
    list(APPEND _cppcheck_args "--enable=warning,style,performance,portability")
    
    # Make it a bit more quiet
    list(APPEND _cppcheck_args "--quiet")
    list(APPEND _cppcheck_args "--suppress=missingIncludeSystem")

    add_custom_command(TARGET
      ${_name}
      PRE_BUILD
      COMMAND
      ${CPPCHECK_EXECUTABLE}
      ${_cppcheck_args}
      ${_includes}
      ${_files}
      WORKING_DIRECTORY
      "${CMAKE_CURRENT_SOURCE_DIR}"
      COMMENT
      "${_name}_cppcheck: Running cppcheck on target ${_name}..."
      VERBATIM)
  endif()
  
endfunction()
