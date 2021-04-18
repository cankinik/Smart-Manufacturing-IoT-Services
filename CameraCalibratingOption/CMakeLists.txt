cmake_minimum_required(VERSION 2.8)
project( CameraCalibration )
find_package( OpenCV REQUIRED )



include_directories(${OpenCV_INCLUDE_DIRS})
include_directories( PRIVATE include)
add_executable( CameraCalibrating main.cpp)
target_link_libraries( CameraCalibrating ${OpenCV_LIBS})

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
include_directories(usr/include/)

