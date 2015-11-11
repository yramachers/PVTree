#Generate script(s) for PVTree installation
if ( NOT(DEFINED CLIMATE_DATA_PATH) )
  message(FATAL_ERROR "CLIMATE_DATA_PATH not defined. Will not be able to find input climate data GRIB files.")
endif ( NOT(DEFINED CLIMATE_DATA_PATH) )

#Environment setup script
set(setupScript_in  ${CMAKE_CURRENT_SOURCE_DIR}/setup.sh.in)
set(setupScript_out ${CMAKE_CURRENT_BINARY_DIR}/setup.sh)

configure_file(${setupScript_in} ${setupScript_out} @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/setup.sh 
        DESTINATION share/scripts/)