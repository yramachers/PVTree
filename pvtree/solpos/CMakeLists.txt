add_library(pvtree-solpos SHARED
  solpos00.h
  solpos.c
  )

add_cppcheck(pvtree-solpos)

#Install results into respective directories
install(TARGETS pvtree-solpos
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib/static)
