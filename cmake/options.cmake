option(WEBVIEW_FETCH_DEPS "Fetch dependencies" ON)

if(WIN32)
    set(WEBVIEW_MSWEBVIEW2_VERSION "1.0.1185.39")
    option(WEBVIEW_MSWEBVIEW2_BUILTIN_IMPL "Enable built-in WebView2Loader implementation" ON)
endif()
