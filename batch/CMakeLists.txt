#Install batch scripts
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
  DESTINATION share
  FILES_MATCHING 
  PATTERN "*.sh"
  PATTERN "*.cfg")
