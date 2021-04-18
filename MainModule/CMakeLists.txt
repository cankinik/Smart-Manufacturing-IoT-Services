cmake_minimum_required(VERSION 2.8)
project( TermProject )
find_package( OpenCV REQUIRED )
find_package(TensorflowCC REQUIRED)


find_package(OpenMP REQUIRED)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()
find_package(Boost COMPONENTS system)
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${PROJECT_BINARY_DIR}/DeepSortFiles/DeepSortCodes/)
include_directories(${PROJECT_BINARY_DIR}/FaceRecognitionFiles/)

file(GLOB_RECURSE SRC_FILES "DeepSortFiles/DeepSortCodes/*.cpp" "DeepSortFiles/DeepSortCodes/*.h" "FaceRecognitionFiles/*.cpp" "FaceRecognitionFiles/*.h")

include_directories( PRIVATE include)
add_executable( TermProject mainProgram.cpp functions.cpp functions.h ${SRC_FILES})
target_link_libraries( TermProject ${OpenCV_LIBS} TensorflowCC::TensorflowCC ${Boost_LIBRARIES})

find_package(CUDA)
if(CUDA_FOUND)
  target_link_libraries(TermProject ${CUDA_LIBRARIES})
endif()

include_directories(usr/include/)
target_link_libraries(TermProject mysqlcppconn)

