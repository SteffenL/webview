cmake_minimum_required(VERSION 3.15)
project(webview)

include(cmake/options.cmake)
include(cmake/build_support.cmake)
include(cmake/deps.cmake)

add_library(webview_common INTERFACE)
add_library(webview_header INTERFACE)
add_library(webview_static STATIC webview.cc)
add_library(webview_shared SHARED webview.cc)

set_target_properties(webview_static PROPERTIES OUTPUT_NAME webview_static)
set_target_properties(webview_shared PROPERTIES
    OUTPUT_NAME webview
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

target_compile_definitions(webview_static PUBLIC WEBVIEW_STATIC PRIVATE WEBVIEW_BUILDING)
target_compile_definitions(webview_shared PUBLIC WEBVIEW_SHARED PRIVATE WEBVIEW_BUILDING)

target_compile_features(webview_common INTERFACE c_std_99 cxx_std_11)

target_include_directories(webview_common INTERFACE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
    $<INSTALL_INTERFACE:.>
)

if(WIN32)
    target_link_libraries(webview_common INTERFACE ole32.lib shell32.lib shlwapi.lib user32.lib)
endif()

target_link_libraries(webview_header INTERFACE webview_common webview_deps)
target_link_libraries(webview_static PUBLIC webview_common webview_deps PRIVATE webview_build_support)
target_link_libraries(webview_shared PUBLIC webview_common PRIVATE webview_deps webview_build_support)

if(ENABLE_TESTS)
    include(CTest)
    add_subdirectory(test)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
