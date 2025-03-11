cmake_minimum_required(VERSION 3.16)
project(webview_amalgamation_test LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

add_subdirectory(../../.. webview)
list(APPEND CMAKE_MODULE_PATH "${WEBVIEW_CURRENT_CMAKE_DIR}/modules")
webview_find_dependencies()

add_executable(webview_amalgamation_test)
target_sources(webview_amalgamation_test PRIVATE main.cc)
target_include_directories(webview_amalgamation_test PRIVATE generated)
target_link_libraries(webview_amalgamation_test PRIVATE ${WEBVIEW_DEPENDENCIES})
target_compile_features(webview_amalgamation_test PRIVATE cxx_std_11)
