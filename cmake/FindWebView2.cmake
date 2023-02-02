set(WebView2_ROOT CACHE STRING "WebView2 SDK root directory")
set(WebView2_FETCH_MISSING TRUE CACHE BOOL "Fetch WebView2 SDK if missing")

set(LOG_TAG "FindWebView2: ")

if(WebView2_ROOT)
    message(STATUS "${LOG_TAG}Root directory: ${WebView2_ROOT}")
    list(APPEND WebView2_INCLUDE_DIR_HINTS ${WebView2_ROOT}/build/native/include)
endif()

find_path(
    WebView2_INCLUDE_DIR
    NAMES WebView2.h
    HINTS ${WebView2_INCLUDE_DIR_HINTS})

mark_as_advanced(WebView2_INCLUDE_DIR)
set(WebView2_INCLUDE_DIRS ${WebView2_INCLUDE_DIR})

if(NOT WebView2_INCLUDE_DIR AND WebView2_FETCH_MISSING)
    if(NOT DEFINED WebView2_FIND_VERSION)
        message(FATAL_ERROR "${LOG_TAG}Please specify a version.")
    endif()
    include(FetchContent)
    set(FC_NAME microsoft_web_webview2)
    FetchContent_Declare(${FC_NAME}
        URL "https://www.nuget.org/api/v2/package/Microsoft.Web.WebView2/${WebView2_FIND_VERSION}"
        CONFIGURE_COMMAND ""
    )
    FetchContent_GetProperties(${FC_NAME})
    if(NOT ${FC_NAME}_POPULATED)
        FetchContent_Populate(${FC_NAME})
        set(WebView2_ROOT ${${FC_NAME}_SOURCE_DIR})
        set(NATIVE_DIR ${WebView2_ROOT}/build/native)
        find_path(
            WebView2_INCLUDE_DIR
            NAMES WebView2.h
            HINTS ${NATIVE_DIR}/include)
        set(WebView2_INCLUDE_DIRS ${WebView2_INCLUDE_DIR})
    endif()
endif()

find_file(
    WebView2_NUSPEC_PATH
    NAMES Microsoft.Web.WebView2.nuspec
    HINTS ${WebView2_ROOT})
mark_as_advanced(WebView2_NUSPEC_PATH)

if(WebView2_NUSPEC_PATH)
    file(READ ${WebView2_NUSPEC_PATH} WebView2_NUSPEC_CONTENT)
    string(REGEX MATCH  "<version>([0-9.]+)" WebView2_FOUND_VERSION_MATCH "${WebView2_NUSPEC_CONTENT}")
    set(WebView2_FOUND_VERSION ${CMAKE_MATCH_1})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    WebView2
    REQUIRED_VARS WebView2_INCLUDE_DIR
    VERSION_VAR WebView2_FOUND_VERSION)

if(WebView2_FOUND)
    if(NOT TARGET WebView2::sdk)
        add_library(WebView2::sdk INTERFACE IMPORTED)
        target_include_directories(WebView2::sdk INTERFACE ${WebView2_INCLUDE_DIRS})
    endif()
endif()
