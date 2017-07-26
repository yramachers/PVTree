#Fill in the GIT version information (so that metadata about version can
#be propogated with the binary + results)

# Access git information using cmake git module
#include(GetGitRevisionDescription)

# Get the current working branch/tag
#git_describe(GIT_BRANCH --all)

# Get the latest abbreviated commit hash of the working branch
#get_git_head_revision(GIT_REFSPEC GIT_COMMIT_HASH)

# Check if any changes are present that need to be committed
#git_changes(GIT_CHANGES_PRESENT)

# Make the variables available
add_definitions("-DGIT_COMMIT_HASH=${GIT_COMMIT_HASH}")
add_definitions("-DGIT_BRANCH=${GIT_BRANCH}")
add_definitions("-DGIT_CHANGES_PRESENT=${GIT_CHANGES_PRESENT}")
set(version_in  ${CMAKE_CURRENT_SOURCE_DIR}/version.hpp.in)
set(version_out ${CMAKE_CURRENT_BINARY_DIR}/version.hpp)
configure_file(${version_in} ${version_out} @ONLY)

include_directories(BEFORE ${PROJECT_SOURCE_DIR})
add_subdirectory(utils)
add_subdirectory(location)
add_subdirectory(geometry)
add_subdirectory(treeSystem)
add_subdirectory(leafSystem)
add_subdirectory(climate)
add_subdirectory(solpos)
add_subdirectory(rootBasedSimulation)
#install(FILES ${CMAKE_CURRENT_BINARY_DIR}/version.hpp
#        DESTINATION include/)
