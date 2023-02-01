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
        add_library(WebView2 INTERFACE GLOBAL)
        target_include_directories(WebView2 INTERFACE ${NATIVE_DIR}/include)
        target_compile_features(WebView2 INTERFACE cxx_std_17)
    endif()
endfunction()
