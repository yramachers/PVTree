#Add the normal include directory and also the generation directory 
#to the include path
include_directories(BEFORE ../include)
include_directories(BEFORE ${CMAKE_CURRENT_BINARY_DIR}/../include)

add_subdirectory(utils)
add_subdirectory(location)
add_subdirectory(geometry)
add_subdirectory(treeSystem)
add_subdirectory(leafSystem)
add_subdirectory(climate)
add_subdirectory(solpos)
add_subdirectory(solarSimulation)
add_subdirectory(material)
add_subdirectory(full)
add_subdirectory(recorders)
add_subdirectory(rootBasedSimulation)
add_subdirectory(analysis)

#This is a little monolithic at the moment... 
set(LIBS ${LIBS} utils location geometry treeSystems leafSystems climate solarSimulation 
  solpos material fullsim recorders rootBasedSimulation analysis)

#Add all the example programs
add_subdirectory(examples)

#Setup unit testing
add_subdirectory(testing)


