include(cmake/toolchain.cmake)

project(webview C CXX)
cmake_minimum_required(VERSION 3.21)

include(cmake/options.cmake)
include(cmake/deps.cmake)
add_subdirectory(src)
add_subdirectory(examples)
add_subdirectory(test)
