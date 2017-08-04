# - Generate HosekSkyModel data file
# Generator program
add_executable(makeHosekSkyModelData makeHosekSkyModelData.cpp ArHosekSkyModelData_Spectral.h)
target_include_directories(makeHosekSkyModelData PRIVATE ${ROOT_INCLUDE_DIRS})
target_link_libraries(makeHosekSkyModelData PRIVATE ${ROOT_Tree_LIBRARY} ${ROOT_RIO_LIBRARY})

# Data file creation
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/HosekSkyModelData.root
  COMMAND makeHosekSkyModelData
  COMMAND ${CMAKE_COMMAND}
          -E copy
          ${CMAKE_CURRENT_BINARY_DIR}/HosekSkyModelData.root
          ${PVTREE_BUILDPRODUCTS_DIR}/${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS makeHosekSkyModelData
  )
add_custom_target(pvtree-hosekskymodel-data ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/HosekSkyModelData.root
  )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/HosekSkyModelData.root
  DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}
  )

