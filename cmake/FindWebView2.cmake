set(WebView2_ROOT CACHE STRING "WebView2 SDK root directory")
set(WebView2_FETCH_MISSING TRUE CACHE BOOL "Fetch WebView2 SDK if missing")

set(LOG_TAG "FindWebView2: ")

if(WebView2_ROOT)
    find_file(
        WebView2_NUSPEC_PATH
        NAMES Microsoft.Web.WebView2.nuspec
        HINTS ${WebView2_ROOT})
    mark_as_advanced(WebView2_NUSPEC_PATH)

    find_path(
        WebView2_INCLUDE_DIR
        NAMES WebView2.h
        HINTS ${WebView2_ROOT}/build/native/include)
    mark_as_advanced(WebView2_INCLUDE_DIR)

    find_path(
        WebView2_WINRT_INCLUDE_DIR
        NAMES WebView2Interop.h
        HINTS ${WebView2_ROOT}/build/native/include-winrt)
    mark_as_advanced(WebView2_WINRT_INCLUDE_DIR)
endif()

set(WebView2_INCLUDE_DIRS ${WebView2_INCLUDE_DIR} ${WebView2_WINRT_INCLUDE_DIR})

if(NOT WebView2_INCLUDE_DIRS AND WebView2_FETCH_MISSING)
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
        set(WebView2_ROOT ${${FC_NAME}_SOURCE_DIR} FORCE)

        find_file(
            WebView2_NUSPEC_PATH
            NAMES Microsoft.Web.WebView2.nuspec
            HINTS ${WebView2_ROOT})
        mark_as_advanced(WebView2_NUSPEC_PATH)
    
        find_path(
            WebView2_INCLUDE_DIR
            NAMES WebView2.h
            HINTS ${WebView2_ROOT}/build/native/include)
        mark_as_advanced(WebView2_INCLUDE_DIR)
    
        find_path(
            WebView2_WINRT_INCLUDE_DIR
            NAMES WebView2Interop.h
            HINTS ${WebView2_ROOT}/build/native/include-winrt)
        mark_as_advanced(WebView2_WINRT_INCLUDE_DIR)

        set(WebView2_INCLUDE_DIRS ${WebView2_INCLUDE_DIR} ${WebView2_WINRT_INCLUDE_DIR})
    endif()
endif()

if(WebView2_NUSPEC_PATH)
    file(READ ${WebView2_NUSPEC_PATH} WebView2_NUSPEC_CONTENT)
    string(REGEX MATCH  "<version>([0-9.]+)" WebView2_FOUND_VERSION_MATCH "${WebView2_NUSPEC_CONTENT}")
    set(WebView2_FOUND_VERSION ${CMAKE_MATCH_1})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    WebView2
    REQUIRED_VARS WebView2_INCLUDE_DIR WebView2_WINRT_INCLUDE_DIR
    VERSION_VAR WebView2_FOUND_VERSION)

if(WebView2_FOUND)
    if(NOT TARGET WebView2::sdk)
        add_library(WebView2::sdk INTERFACE IMPORTED)
        target_include_directories(WebView2::sdk INTERFACE ${WebView2_INCLUDE_DIRS})
    endif()
    if(NOT TARGET WebView2::loader)
        if(CMAKE_SIZEOF_VOID_P MATCHES 4)
            set(LIB_SUBDIR "x86")
        else()
            set(LIB_SUBDIR "x64")
        endif()

        add_library(WebView2::loader SHARED IMPORTED)
        target_link_libraries(WebView2::loader INTERFACE WebView2::sdk)
        set_target_properties(WebView2::loader PROPERTIES
            IMPORTED_IMPLIB ${WebView2_ROOT}/${LIB_SUBDIR}/WebView2Loader.dll.lib
            IMPORTED_LOCATION ${WebView2_ROOT}/${LIB_SUBDIR}/WebView2Loader.dll
        )
    endif()
endif()
