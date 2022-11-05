cmake_minimum_required(VERSION 3.21)

# Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

include(cmake/toolchain.cmake)
project(webview C CXX)

include(cmake/build_support.cmake)
include(cmake/deps.cmake)

add_subdirectory(src)
add_subdirectory(examples)
add_subdirectory(test)
