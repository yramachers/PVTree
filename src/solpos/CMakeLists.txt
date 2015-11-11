add_library(solpos solpos.c)

add_cppcheck(solpos)

#Install results into respective directories
install(TARGETS solpos
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib/static)