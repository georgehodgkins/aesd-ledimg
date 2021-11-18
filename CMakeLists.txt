cmake_minimum_required(VERSION 2.8)
project( ledimg )
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

#add_library( ledgrid OBJECT ledgrid.c )
#add_executable( gridtest gridtest.c $<TARGET_OBJECTS:ledgrid> )

add_executable( testapp dummy.cpp )
target_link_libraries( testapp ${OpenCV_LIBS} )

install(TARGETS testapp DESTINATION ./bin)

