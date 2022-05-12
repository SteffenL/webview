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
