cmake_minimum_required(VERSION 2.8)
project( ledimg )
find_package( OpenCV REQUIRED )
set( CMAKE_BUILD_TYPE Debug )
set( CMAKE_CXX_STANDARD 17 )
#find_library( libgpiod libgpiod.so REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

add_library( ledgrid OBJECT ledgrid.c )
add_executable( gridtest gridtest.c $<TARGET_OBJECTS:ledgrid> )
target_link_libraries( gridtest gpiod )

add_executable( circle circle.cpp $<TARGET_OBJECTS:ledgrid> )
target_link_libraries( circle ${OpenCV_LIBS} )

install(TARGETS circle gridtest DESTINATION ./bin)

