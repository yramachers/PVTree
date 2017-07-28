#Install default configuration files
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}
  PATTERN "CMakeLists.txt" EXCLUDE
  )
