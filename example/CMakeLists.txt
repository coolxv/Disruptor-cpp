# DisruptorDemo CMakeLists.txt
cmake_minimum_required(VERSION 2.6)
project(DisruptorDemo)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

include_directories(${CMAKE_SOURCE_DIR}/..)
link_directories(${CMAKE_SOURCE_DIR}/../build/Disruptor)

add_executable(DisruptorDemo ${CMAKE_SOURCE_DIR}/DisruptorDemo.cpp)
target_link_libraries(DisruptorDemo Disruptor pthread)