cmake_minimum_required(VERSION 3.7)

project(webserver LANGUAGES CXX)

# require C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_VERBOSE_MAKEFILE ON)  # 显示详细编译输出
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
add_definitions(-DUNIT_TEST)

add_subdirectory(test)

# file(GLOB ./*.cc SRC_LIST)
add_executable(webserver main.cc util.cc)
