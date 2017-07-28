# add a target to generate API documentation with Doxygen
find_package(Doxygen)
option(BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" ${DOXYGEN_FOUND})

if(BUILD_DOCUMENTATION)
  if(NOT DOXYGEN_FOUND)
    message(FATAL_ERROR "Doxygen is needed to build the documentation.")
  endif()

  set(doxy_main_page ${PROJECT_SOURCE_DIR}/README.md)
  set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
  set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

  configure_file(${doxyfile_in} ${doxyfile} @ONLY)

  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/html/index.html
    COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen"
    DEPENDS
      ${doxy_main_page}
      ${doxyfile_in}
      # List targets explicitly as we want docs to be rebuild if inputs change
      pvtree-analysis
      pvtree-climate
      pvtree-fullsim
      pvtree-geometry
      pvtree-leafSystems
      pvtree-location
      pvtree-rootBasedSimulation
      pvtree-solpos
      pvtree-treeSystems
      pvtree-utils
    VERBATIM
    )
  add_custom_target(doc ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/html/index.html)
  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION ${CMAKE_INSTALL_DOCDIR})
endif()
