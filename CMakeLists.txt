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

project(webview
    VERSION "${WEBVIEW_VERSION_MAJOR}.${WEBVIEW_VERSION_MINOR}.${WEBVIEW_VERSION_PATCH}"
    LANGUAGES C CXX)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

include(CMakePackageConfigHelpers)
include(GenerateExportHeader)
include(GNUInstallDirs)

include(cmake/options.cmake)

add_subdirectory(build_support)
add_subdirectory(core)
add_subdirectory(examples)
add_subdirectory(test)

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
    EXPORT webview
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
install(EXPORT webview
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/webview
    NAMESPACE webview::
    COMPONENT webview_cmake_package
)
