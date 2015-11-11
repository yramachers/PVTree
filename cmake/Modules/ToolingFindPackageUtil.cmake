# /ToolingFindPackageUtil.cmake
#
# Some helper functions for using find_package to find an executable tool to
# run over a codebase. Sets up and verifies versions, requirements, etc.
#
# See LICENCE.md for Copyright information

include (CMakeParseArguments)
include (FindPackageHandleStandardArgs)
include (FindPackageMessage)

# psq_print_if_not_quiet
#
# Print a message as specified as part of ARGN
# as long as ${PREFIX}_FIND_QUIETLY is not set
#
# PREFIX: The package prefix passed to find_package for this module
# MSG: Message to print
# DEPENDS: Variables, which when changed, the message should be re-displayed
function (psq_print_if_not_quiet PREFIX)

    set (PRINT_IF_NOT_QUIET_MULTIVAR_ARGS MSG DEPENDS)
    cmake_parse_arguments (PRINT_IF_NOT_QUIET
                           ""
                           ""
                           "${PRINT_IF_NOT_QUIET_MULTIVAR_ARGS}"
                           ${ARGN})

    string (REPLACE ";" " " MSG "${PRINT_IF_NOT_QUIET_MSG}")

    set (DEPEND_VARS_STRING "")
    foreach (DEPEND_VAR ${DEPENDS})

        set (DEPEND_VARS_STRING "${DEPEND_VARS_STRING}[${${DEPEND_VAR}}]")

    endforeach ()

    if (DEPEND_VARS_STRING)

        find_package_message (${PREFIX} "${MSG}" "${DEPEND_VARS_STRING}")

    else (DEPEND_VARS_STRING)

        if (NOT ${PREFIX}_FIND_QUIETLY)

            message (STATUS "${MSG}")

        endif (NOT ${PREFIX}_FIND_QUIETLY)

    endif (DEPEND_VARS_STRING)

endfunction ()

# psq_report_not_found_if_not_quiet
#
# If ${PREFIX}_FIND_QUIETLY is not set, print the error message as
# specified in ARGN if the variable named VARIABLE is not set.
#
# PREFIX: The package prefix passed to find_package for this module
# VARIABLE: The name of the variable to test
function (psq_report_not_found_if_not_quiet PREFIX VARIABLE)

    if (NOT ${VARIABLE})

        psq_print_if_not_quiet (${PREFIX} MSG ${ARGN})

    endif (NOT ${VARIABLE})

endfunction ()

function (_psq_find_tool_executable_in_custom_paths EXECUTABLE_TO_FIND
                                                    PATH_RETURN)

    set (FIND_TOOL_EXECUTABLE_CUSTOM_PATHS_MULTIVAR_ARGS PATHS PATH_SUFFIXES)
    cmake_parse_arguments (FIND_TOOL_EXECUTABLE_CUSTOM_PATHS
                           ""
                           ""
                           "${FIND_TOOL_EXECUTABLE_CUSTOM_PATHS_MULTIVAR_ARGS}"
                           ${ARGN})

    unset (PATH_TO_EXECUTABLE CACHE)
    find_program (PATH_TO_EXECUTABLE
                  ${EXECUTABLE_TO_FIND}
                  PATHS ${FIND_TOOL_EXECUTABLE_CUSTOM_PATHS}
                  PATH_SUFFIXES ${FIND_TOOL_EXECUTABLE_PATH_SUFFIXES}
                  NO_DEFAULT_PATH)

    if (PATH_TO_EXECUTABLE)

        set (${PATH_RETURN} ${PATH_TO_EXECUTABLE} PARENT_SCOPE)

    endif (PATH_TO_EXECUTABLE)

endfunction (_psq_find_tool_executable_in_custom_paths)

function (_psq_find_tool_executable_in_system_paths EXECUTABLE_TO_FIND
                                                    PATH_RETURN)

    unset (PATH_TO_EXECUTABLE CACHE)
    find_program (PATH_TO_EXECUTABLE ${EXECUTABLE_TO_FIND})

    if (PATH_TO_EXECUTABLE)

        set (${PATH_RETURN} ${PATH_TO_EXECUTABLE} PARENT_SCOPE)

    endif (PATH_TO_EXECUTABLE)

endfunction (_psq_find_tool_executable_in_system_paths)

# psq_find_tool_executable
#
# Finds the executable EXECUTABLE_TO_FIND and places the result in PATH_RETURN
#
# EXECUTABLE_TO_FIND: The name of the executable to find
# PATH_RETURN: A variable to place the full path when found
# [Optional] CUSTOM_PATHS: Paths to search first before searching system paths
# [Optional] PATH_SUFFIXES: Suffixes on each installation root (eg, bin)
function (psq_find_tool_executable EXECUTABLE_TO_FIND PATH_RETURN)

    set (FIND_TOOL_EXECUTABLE_MULTIVAR_ARGS CUSTOM_PATHS PATH_SUFFIXES)
    cmake_parse_arguments (FIND_TOOL_EXECUTABLE
                           ""
                           ""
                           "${FIND_TOOL_EXECUTABLE_MULTIVAR_ARGS}"
                           ${ARGN})

    unset (PATH_TO_EXECUTABLE CACHE)
    if (FIND_TOOL_EXECUTABLE_CUSTOM_PATHS)

        set (PATHS ${FIND_TOOL_EXECUTABLE_CUSTOM_PATHS})
        set (PATH_SUFFIXES ${FIND_TOOL_EXECUTABLE_PATH_SUFFIXES})
        _psq_find_tool_executable_in_custom_paths (${EXECUTABLE_TO_FIND}
                                                   PATH_TO_EXECUTABLE
                                                   PATHS
                                                   ${PATHS}
                                                   PATH_SUFFIXES
                                                   ${PATH_SUFFIXES})

    endif (FIND_TOOL_EXECUTABLE_CUSTOM_PATHS)

    if (NOT PATH_TO_EXECUTABLE)

        _psq_find_tool_executable_in_system_paths (${EXECUTABLE_TO_FIND}
                                                   PATH_TO_EXECUTABLE)

    endif (NOT PATH_TO_EXECUTABLE)

    if (PATH_TO_EXECUTABLE)

        set (${PATH_RETURN} ${PATH_TO_EXECUTABLE} PARENT_SCOPE)

    endif (PATH_TO_EXECUTABLE)

endfunction (psq_find_tool_executable)

# psq_find_tool_extract_version
#
# Runs the tool and fetches its version, placing the result into VERSION_RETURN
#
# TOOL_EXECUTABLE: The path to the tool
# VERSION_RETURN: A variable to place the full version when detected
# [Optional] VERSION_ARG: Argument to pass to the tool when running it to
#                         fetch its version.
# [Optional] VERSION_HEADER: Text that comes before the version number.
# [Optional] VERSION_END_TOKEN: Text that comes after the version number.
function (psq_find_tool_extract_version TOOL_EXECUTABLE VERSION_RETURN)

    set (FIND_TOOL_EXTRACT_VERSION_SINGLEVAR_ARGS
         VERSION_ARG
         VERSION_HEADER
         VERSION_END_TOKEN)
    cmake_parse_arguments (FIND_TOOL
                           ""
                           "${FIND_TOOL_EXTRACT_VERSION_SINGLEVAR_ARGS}"
                           ""
                           ${ARGN})

    execute_process (COMMAND ${TOOL_EXECUTABLE}
                     ${FIND_TOOL_VERSION_ARG}
                     OUTPUT_VARIABLE TOOL_VERSION_OUTPUT)

    if (FIND_TOOL_VERSION_HEADER)

        string (FIND "${TOOL_VERSION_OUTPUT}" "${FIND_TOOL_VERSION_HEADER}" 
                FIND_TOOL_VHEADER_LOC)
        string (LENGTH "${FIND_TOOL_VERSION_HEADER}"
                FIND_TOOL_VHEADER_SIZE)
        math (EXPR FIND_TOOL_VERSION_START
              "${FIND_TOOL_VHEADER_LOC} + ${FIND_TOOL_VHEADER_SIZE}")
        string (SUBSTRING "${TOOL_VERSION_OUTPUT}"
                ${FIND_TOOL_VERSION_START} -1 FIND_TOOL_VERSION_TO_END)

    else (FIND_TOOL_VERSION_HEADER)

        set (FIND_TOOL_VERSION_TO_END ${TOOL_VERSION_OUTPUT})

    endif (FIND_TOOL_VERSION_HEADER)

    if (FIND_TOOL_VERSION_END_TOKEN)

        string (FIND "${FIND_TOOL_VERSION_TO_END}" "${FIND_TOOL_VERSION_END_TOKEN}"
                FIND_TOOL_RETURN_LOC)
        string (SUBSTRING "${FIND_TOOL_VERSION_TO_END}" 
                0 ${FIND_TOOL_RETURN_LOC} FIND_TOOL_VERSION)

    else (FIND_TOOL_VERSION_END_TOKEN)

        set (FIND_TOOL_VERSION ${FIND_TOOL_VERSION_TO_END})

    endif (FIND_TOOL_VERSION_END_TOKEN)

    if (NOT FIND_TOOL_VERSION)

        message (FATAL_ERROR "Failed to find tool version by executing "
                             "${TOOL_EXECUTABLE} ${FIND_TOOL_VERSION_ARG} "
                             "and splicing between the header "
                             "'${FIND_TOOL_VERSION_HEADER}' and footer "
                             "'${FIND_TOOL_VERSION_END_TOKEN}'. The output to "
                             "scan was ${TOOL_VERSION_OUTPUT}")

    endif (NOT FIND_TOOL_VERSION)

    # Strip out any \n
    string (REPLACE "\n" "" FIND_TOOL_VERSION "${FIND_TOOL_VERSION}")

    set (${VERSION_RETURN} ${FIND_TOOL_VERSION} PARENT_SCOPE)

endfunction ()

# psq_check_and_report_tool_version
#
# For the package specified by PREFIX, determines if the detected
# VERSION matched the requested version passed to find_package. If not
# and we are not finding quietly, report problems. If the version check
# is satisfied and all REQUIRED_VARS are set, then set each of the
# REQUIRED_VARS in the PARENT_SCOPE
#
# PREFIX: The package prefix passed to find_package for this module
# VERSION: The detected tool version
# REQUIRED_VARS : Required variables, set in parent scope if present
#                 Must specify at least one.
macro (psq_check_and_report_tool_version PREFIX VERSION)

    string (STRIP "${VERSION}" VERSION)

    cmake_parse_arguments (_PSQ_CHECK_${PREFIX}
                           ""
                           ""
                           "REQUIRED_VARS"
                           ${ARGN})

    find_package_handle_standard_args (${PREFIX}
                                       FOUND_VAR ${PREFIX}_FOUND
                                       REQUIRED_VARS
                                       ${_PSQ_CHECK_${PREFIX}_REQUIRED_VARS}
                                       VERSION_VAR VERSION)

    if (${PREFIX}_FOUND)

        foreach (VARIABLE ${_PSQ_CHECK_${PREFIX}_REQUIRED_VARS})

            set (${VARIABLE} ${${VARIABLE}} PARENT_SCOPE)

        endforeach ()

    endif (${PREFIX}_FOUND)

endmacro (psq_check_and_report_tool_version)

# psq_find_executable_installation_root
#
# For the path to a TOOL_EXECUTABLE, get the installation prefix
# of that executable and place it in the variable named by INSTALL_ROOT_RETURN
#
# TOOL_EXECUTABLE: Path to an executable
# INSTALL_ROOT_RETURN: A variable to place the full path to install root
# [Optional] PREFIX_SUBDIRECTORY: A partial path of directories between the
#                                 executable itself and install root
#                                 (eg /bin/)
function (psq_find_executable_installation_root TOOL_EXECUTABLE
                                                INSTALL_ROOT_RETURN)

    get_filename_component (TOOL_EXEC_PATH ${TOOL_EXECUTABLE} ABSOLUTE)
    get_filename_component (TOOL_EXEC_BASE ${TOOL_EXECUTABLE} NAME)

    set (INSTALL_ROOT_SINGLEVAR_ARGS PREFIX_SUBDIRECTORY)
    cmake_parse_arguments (INSTALL_ROOT
                           ""
                           "${INSTALL_ROOT_SINGLEVAR_ARGS}"
                           ""
                           ${ARGN})

    # Strip unsanitised string
    string (STRIP ${TOOL_EXEC_PATH} TOOL_EXEC_PATH)

    # First get the tool path lengths
    string (LENGTH "${TOOL_EXEC_PATH}"
            TOOL_EXEC_PATH_LENGTH)
    string (LENGTH "/${INSTALL_ROOT_PREFIX_SUBDIRECTORY}/${TOOL_EXEC_BASE}"
            TOOL_EXEC_SUBDIR_LENGTH)

    # Then determine how long the prefix is
    math (EXPR TOOL_EXEC_PREFIX_LENGTH
          "${TOOL_EXEC_PATH_LENGTH} - ${TOOL_EXEC_SUBDIR_LENGTH}")

    # Then we get the prefix substring
    string (SUBSTRING ${TOOL_EXEC_PATH} 0 ${TOOL_EXEC_PREFIX_LENGTH}
            TOOL_INSTALL_ROOT)

    set (${INSTALL_ROOT_RETURN} ${TOOL_INSTALL_ROOT} PARENT_SCOPE)

endfunction (psq_find_executable_installation_root)

# psq_find_path_in_installation_root
#
# Places the full path to SUBDIRECTORY_TO_FIND in PATH_RETURN if found in 
# INSTALL_ROOT
#
# INSTALL_ROOT: The directory to search for
# SUBDIRECTORY_TO_FIND: The name of the subdirectory to find
# PATH_RETURN: A variable to place the full path when found
function (psq_find_path_in_installation_root INSTALL_ROOT
                                             SUBDIRECTORY_TO_FIND
                                             PATH_RETURN)

    find_path (_PATH
               ${SUBDIRECTORY_TO_FIND}
               PATHS ${INSTALL_ROOT}
               NO_DEFAULT_PATH)

    if (_PATH)

        mark_as_advanced (_PATH)
        set (${PATH_RETURN} ${_PATH}/${SUBDIRECTORY_TO_FIND} PARENT_SCOPE)
        unset (_PATH CACHE)

    endif (_PATH)

endfunction (psq_find_path_in_installation_root)