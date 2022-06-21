function(fetch_webview2 VERSION)
    message("Fetching WebView2 ${VERSION}")
    include(FetchContent)
    set(FC_NAME microsoft_web_webview2)
    FetchContent_Declare(${FC_NAME}
        URL "https://www.nuget.org/api/v2/package/Microsoft.Web.WebView2/${VERSION}"
        CONFIGURE_COMMAND ""
    )
    FetchContent_GetProperties(${FC_NAME})
    if(NOT ${FC_NAME}_POPULATED)
        FetchContent_Populate(${FC_NAME})
        set(${FC_NAME}_SOURCE_DIR ${${FC_NAME}_SOURCE_DIR} CACHE STRING "" FORCE)
        mark_as_advanced(${FC_NAME}_SOURCE_DIR)
        set(NATIVE_DIR ${${FC_NAME}_SOURCE_DIR}/build/native)

        if(CMAKE_SIZEOF_VOID_P MATCHES 4)
            set(LIB_SUBDIR "x86")
        else()
            set(LIB_SUBDIR "x64")
        endif()

        add_library(WebView2LoaderStatic STATIC IMPORTED GLOBAL)
        set_target_properties(WebView2LoaderStatic PROPERTIES
            IMPORTED_LOCATION ${NATIVE_DIR}/${LIB_SUBDIR}/WebView2LoaderStatic.lib
        )
        target_include_directories(WebView2LoaderStatic INTERFACE ${NATIVE_DIR}/include)
        target_compile_features(WebView2LoaderStatic INTERFACE cxx_std_17)

        add_library(WebView2Loader SHARED IMPORTED GLOBAL)
        set_target_properties(WebView2Loader PROPERTIES
            IMPORTED_IMPLIB ${NATIVE_DIR}/${LIB_SUBDIR}/WebView2Loader.dll.lib
            IMPORTED_LOCATION ${NATIVE_DIR}/${LIB_SUBDIR}/WebView2Loader.dll
        )
        target_include_directories(WebView2Loader INTERFACE ${NATIVE_DIR}/include)
        target_compile_features(WebView2Loader INTERFACE cxx_std_17)
    endif()
endfunction()

set(TARGET_NAME webview_deps)
add_library(${TARGET_NAME} INTERFACE)

if(WIN32)
    if(WEBVIEW2_SHARED)
        set(WEBVIEW2_LIBRARY WebView2Loader)
    elseif(MSVC)
        set(WEBVIEW2_LIBRARY WebView2LoaderStatic)
    else()
        message(WARNING "WebView2 static library cannot be used with this compiler; using shared library instead.")
        set(WEBVIEW2_LIBRARY WebView2Loader)
    endif()
    if(NOT TARGET ${WEBVIEW2_LIBRARY})
        fetch_webview2(${WEBVIEW2_VERSION})
    endif()
    target_link_libraries(${TARGET_NAME} INTERFACE ${WEBVIEW2_LIBRARY})
    target_compile_definitions(${TARGET_NAME} INTERFACE WEBVIEW_EDGE)
elseif(APPLE)
    find_library(WEBKIT_LIBRARY WebKit REQUIRED)
    target_link_libraries(${TARGET_NAME} INTERFACE ${WEBKIT_LIBRARY})
    target_compile_definitions(${TARGET_NAME} INTERFACE WEBVIEW_COCOA)
elseif(UNIX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK REQUIRED gtk+-3.0)
    pkg_check_modules(WEBKIT2GTK REQUIRED webkit2gtk-4.0)
    target_include_directories(${TARGET_NAME} INTERFACE include ${WEBKIT2GTK_INCLUDE_DIRS})
    target_link_libraries(${TARGET_NAME} INTERFACE ${WEBKIT2GTK_LIBRARIES} pthread)
    target_compile_definitions(${TARGET_NAME} INTERFACE WEBVIEW_GTK)
endif()
