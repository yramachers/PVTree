#Install the spectra
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
  DESTINATION share
  FILES_MATCHING PATTERN "*.results")
