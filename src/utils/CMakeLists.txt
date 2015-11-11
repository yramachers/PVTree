add_library(utils SHARED getRSS.cpp getopt_pp.cpp signalReceiver.cpp)

add_cppcheck(utils)

#Install results into respective directories
install(TARGETS utils
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib/static)