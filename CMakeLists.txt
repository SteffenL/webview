cmake_minimum_required(VERSION 3.21)

# Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

# Read version from the header's header file.
file(READ core/include/webview/webview.h WEBVIEW_H_CONTENT)
string(REGEX MATCH "#define WEBVIEW_VERSION_MAJOR ([0-9]+)" WEBVIEW_VERSION_MATCH "${WEBVIEW_H_CONTENT}")
set(WEBVIEW_VERSION_MAJOR ${CMAKE_MATCH_1})
string(REGEX MATCH "#define WEBVIEW_VERSION_MINOR ([0-9]+)" WEBVIEW_VERSION_MATCH "${WEBVIEW_H_CONTENT}")
set(WEBVIEW_VERSION_MINOR ${CMAKE_MATCH_1})
string(REGEX MATCH "#define WEBVIEW_VERSION_PATCH ([0-9]+)" WEBVIEW_VERSION_MATCH "${WEBVIEW_H_CONTENT}")
set(WEBVIEW_VERSION_PATCH ${CMAKE_MATCH_1})
set(WEBVIEW_VERSION ${WEBVIEW_VERSION_MAJOR}.${WEBVIEW_VERSION_MINOR}.${WEBVIEW_VERSION_PATCH})

project(webview
    VERSION ${WEBVIEW_VERSION}
    LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(CMakePackageConfigHelpers)
include(GenerateExportHeader)
include(GNUInstallDirs)

include(cmake/options.cmake)

add_subdirectory(build_support)
add_subdirectory(core)
add_subdirectory(examples)
add_subdirectory(test)

set(WEBVIEW_CMAKE_MODULES_DIR cmake)

configure_package_config_file(
    cmake/WebviewConfig.cmake.in WebviewConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/webview
    PATH_VARS WEBVIEW_CMAKE_MODULES_DIR
    NO_SET_AND_CHECK_MACRO
    NO_CHECK_REQUIRED_COMPONENTS_MACRO)

write_basic_package_version_file(
    WebviewConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

install(
    TARGETS
        webview_build_support
        webview_core_base
        webview_core_header
        webview_core_shared
        webview_core_static
    EXPORT webview_targets
    RUNTIME
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT webview_runtime
    LIBRARY
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT webview_runtime
        NAMELINK_COMPONENT webview_development
    ARCHIVE
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT webview_development
)

install(
    FILES
        cmake/FindWebView2.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/WebviewConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/WebviewConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/webview
    COMPONENT webview_cmake_package)

install(FILES core/include/webview.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT webview_development)

install(DIRECTORY core/include/webview
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT webview_development)

install(EXPORT webview_targets
    FILE WebviewTargets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/webview
    NAMESPACE webview::
    COMPONENT webview_cmake_package)
