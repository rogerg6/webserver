cmake_minimum_required(VERSION 3.7)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
# file(GLOB SOURCES unittest/*.cc)

add_executable(test_request test_request.cc)