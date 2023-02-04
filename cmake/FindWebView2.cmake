# Input variables:
#   WebView2_ROOT          - WebView2 SDK root directory
#   WebView2_FETCH_MISSING - Fetch WebView2 SDK if missing
# Targets:
#   WebView2::sdk          - WebView2 SDK headers
#   WebView2::loader       - Shared WebView2 loader
#   WebView2::loader_s     - Static WebView2 loader

# Set default values for undefined variables.
if(NOT DEFINED WebView2_FETCH_MISSING)
    set(WebView2_FETCH_MISSING TRUE)
endif()

# Normalize input variables.
if(WebView2_ROOT)
    cmake_path(NORMAL_PATH WebView2_ROOT)
endif()

# Download WebView2 SDK if desired.
if(NOT WebView2_ROOT AND WebView2_FETCH_MISSING)
    if(NOT DEFINED WebView2_FIND_VERSION)
        message(FATAL_ERROR "Please specify a WebView2 SDK version")
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
        set(WebView2_ROOT ${${FC_NAME}_SOURCE_DIR} CACHE PATH "WebView2 SDK root directory" FORCE)
    endif()
endif()

if(WebView2_ROOT)
    # Reset cache variables when the root directory changes.
    if(WebView2_CURRENT_ROOT)
        if(NOT WebView2_ROOT STREQUAL WebView2_CURRENT_ROOT)
            unset(WebView2_NUSPEC_PATH CACHE)
            unset(WebView2_INCLUDE_DIR CACHE)
            unset(WebView2_WINRT_INCLUDE_DIR CACHE)
            set(WebView2_CURRENT_ROOT ${WebView2_ROOT} CACHE PATH "" FORCE)
            mark_as_advanced(WebView2_CURRENT_ROOT)
        endif()
    else()
        set(WebView2_CURRENT_ROOT ${WebView2_ROOT} CACHE PATH "" FORCE)
        mark_as_advanced(WebView2_CURRENT_ROOT)
    endif()

    set(NATIVE_DIR ${WebView2_ROOT}/build/native)

    find_file(
        WebView2_NUSPEC_PATH
        NAMES Microsoft.Web.WebView2.nuspec
        HINTS ${WebView2_ROOT})
    mark_as_advanced(WebView2_NUSPEC_PATH)

    find_path(
        WebView2_INCLUDE_DIR
        NAMES WebView2.h
        HINTS ${NATIVE_DIR}/include)
    mark_as_advanced(WebView2_INCLUDE_DIR)

    find_path(
        WebView2_WINRT_INCLUDE_DIR
        NAMES WebView2Interop.h
        HINTS ${NATIVE_DIR}/include-winrt)
    mark_as_advanced(WebView2_WINRT_INCLUDE_DIR)

    set(WebView2_INCLUDE_DIRS ${WebView2_INCLUDE_DIR} ${WebView2_WINRT_INCLUDE_DIR})
endif()

# Extract version number from the *.nuspec file.
if(WebView2_NUSPEC_PATH)
    file(READ ${WebView2_NUSPEC_PATH} WebView2_NUSPEC_CONTENT)
    string(REGEX MATCH  "<version>([0-9.]+)" WebView2_FOUND_VERSION_MATCH "${WebView2_NUSPEC_CONTENT}")
    set(WebView2_FOUND_VERSION ${CMAKE_MATCH_1})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    WebView2
    REQUIRED_VARS WebView2_ROOT WebView2_INCLUDE_DIRS
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

        add_library(WebView2::loader_s STATIC IMPORTED)
        target_link_libraries(WebView2::loader_s INTERFACE WebView2::sdk)
        set_target_properties(WebView2::loader_s PROPERTIES
            IMPORTED_LOCATION ${NATIVE_DIR}/${LIB_SUBDIR}/WebView2LoaderStatic.lib
        )

        add_library(WebView2::loader SHARED IMPORTED)
        target_link_libraries(WebView2::loader INTERFACE WebView2::sdk)
        set_target_properties(WebView2::loader PROPERTIES
            IMPORTED_IMPLIB ${NATIVE_DIR}/${LIB_SUBDIR}/WebView2Loader.dll.lib
            IMPORTED_LOCATION ${NATIVE_DIR}/${LIB_SUBDIR}/WebView2Loader.dll
        )
    endif()
endif()
