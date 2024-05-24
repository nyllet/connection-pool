cmake_minimum_required(VERSION 3.25.1)
project(connection_pool)
set(CMAKE_VERBOSE_MAKEFILE on)
include_directories($ENV{GMOCK_HOME}/include $ENV{GMOCK_HOME}/gtest/include)
link_directories($ENV{GMOCK_HOME}/mybuild $ENV{GMOCK_HOME}/gtest/mybuild)
add_definitions(-std=c++26 --coverage)
set(CMAKE_CXX_FLAGS "${CMAXE_CXX_FLAGS} -Wall -Wextra -Werror -fprofile-arcs -ftest-coverage")
set(CMAKE_EXE_LINKER_FLAGS "-fprofile-arcs -ftest-coverage")
set(sources 
   main.cpp
   ConnectionPool.hpp
   DummyConnection.hpp)

set(test_sources
   ConnectionPoolTest.cpp 
   MultiThreadConnectionPoolTest.cpp)
add_executable(test ${test_sources} ${sources})
target_link_libraries(test pthread)
target_link_libraries(test gmock)
target_link_libraries(test gtest)
target_link_libraries(test gcov)
