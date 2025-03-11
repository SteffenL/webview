cmake_minimum_required(VERSION 3.16)
project(webview_amalgamation_test LANGUAGES CXX)

add_subdirectory(../../.. webview)
webview_find_dependencies()

add_executable(webview_amalgamation_test)
target_sources(webview_amalgamation_test PRIVATE main.cc)
target_include_directories(webview_amalgamation_test PRIVATE ../generated)
target_link_libraries(webview_amalgamation_test PRIVATE ${WEBVIEW_DEPENDENCIES})
