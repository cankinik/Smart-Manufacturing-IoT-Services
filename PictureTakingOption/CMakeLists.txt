cmake_minimum_required(VERSION 2.8)
project( Picture )
find_package( OpenCV REQUIRED )



include_directories(${OpenCV_INCLUDE_DIRS})
include_directories( PRIVATE include)
add_executable( PictureTaking main.cpp)
target_link_libraries( PictureTaking ${OpenCV_LIBS})

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
include_directories(usr/include/)

