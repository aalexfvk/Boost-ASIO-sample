cmake_minimum_required(VERSION 3.0.0)
project(client VERSION 0.1.0)

SET(CMAKE_CXX_FLAGS -pthread)
add_executable(client client.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
