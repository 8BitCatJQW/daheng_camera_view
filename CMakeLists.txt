cmake_minimum_required(VERSION 3.1)

if (CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "-O3 -Wall")
endif()


#set(LIBS ${OpenCV_LIBS} ${EIGEN3_LIBS} ${Pangolin_LIBRARIES} -lboost_system)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CUDA_ARCH "-arch=sm_50" CACHE STRING "Value of the NVCC -arch option.")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")

set(CMAKE_FIND_PACKAGE_SORT_ORDER NATURAL)


include_directories(${Opencv_INCLUDE_DIRS})
link_directories(${Opencv_LIBRARIES_DIRS})

find_package(CUDA REQUIRED)
find_package(OpenCV REQUIRED)


include_directories(${OpenCV_INCLUDE_DIRS})
find_package(Threads)
#find_package(OpenCV REQUIRED core)

add_compile_options(-std=c++11)

include_directories(/include)


FILE(GLOB_RECURSE CPP_SRC_LIST "src/*.cpp")

cuda_add_executable(stereo_movie main.cpp ${CPP_SRC_LIST})
target_link_libraries(stereo_movie sgm ${CUDA_LIBRARIES} ${OpenCV_LIBS} pthread)

target_link_libraries( stereo_movie /usr/lib/libgxiapi.so)
  


