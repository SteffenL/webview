include(CMakeDependentOption)

cmake_dependent_option(WEBVIEW_USE_STATIC_RUNTIME "Link standard runtime libraries statically" OFF WIN32 OFF)

if(WIN32)
    set(WEBVIEW_MSWEBVIEW2_VERSION 1.0.1185.39 CACHE STRING "WebView2 SDK version")
endif()

cmake_dependent_option(WEBVIEW_MSWEBVIEW2_BUILTIN_IMPL "Enable built-in WebView2Loader implementation" ON WIN32 OFF)
cmake_dependent_option(WEBVIEW_MSWEBVIEW2_EXPLICIT_LINK "Link WebView2Loader.dll explicitly" ${WEBVIEW_MSWEBVIEW2_BUILTIN_IMPL} WIN32 OFF)
cmake_dependent_option(WEBVIEW_MSWEBVIEW2_USE_STATIC_LIBRARY "Link WebView2Loader statically instead of dynamically" OFF WIN32 OFF)
