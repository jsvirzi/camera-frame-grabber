cmake_minimum_required(VERSION 3.20)
project(frame_grabber)

set(CMAKE_CXX_STANDARD 14)

#include_directories(/usr/local/include/opencv4)

#add_executable(frame_grabber src/main.cpp)

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
add_executable(frame_grabber src/main.cpp )
target_link_libraries(frame_grabber ${OpenCV_LIBS})