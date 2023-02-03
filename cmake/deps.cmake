add_library(webview_deps INTERFACE)

if(WIN32)
    find_package(WebView2 ${WEBVIEW_MSWEBVIEW2_VERSION} REQUIRED)
    set(MSWEBVIEW2_LIBRARY WebView2::sdk)
    if(NOT WEBVIEW_MSWEBVIEW2_BUILTIN_IMPL)
        if(WEBVIEW_MSWEBVIEW2_USE_STATIC_LIBRARY)
            set(MSWEBVIEW2_LIBRARY WebView2::loader_s)
        elseif(NOT WEBVIEW_MSWEBVIEW2_EXPLICIT_LINK)
            set(MSWEBVIEW2_LIBRARY WebView2::loader)
        endif()
    endif()
    target_link_libraries(webview_deps INTERFACE ${MSWEBVIEW2_LIBRARY})
    target_compile_definitions(webview_deps INTERFACE WEBVIEW_EDGE)
    target_compile_features(webview_deps INTERFACE cxx_std_17)
elseif(APPLE)
    find_library(WEBKIT_LIBRARY WebKit REQUIRED)
    target_link_libraries(webview_deps INTERFACE ${WEBKIT_LIBRARY})
    target_compile_definitions(webview_deps INTERFACE WEBVIEW_COCOA)
elseif(UNIX)
    find_package(PkgConfig REQUIRED)
    find_package(Threads REQUIRED)
    pkg_check_modules(GTK REQUIRED gtk+-3.0)
    pkg_check_modules(WEBKIT2GTK REQUIRED webkit2gtk-4.0)
    target_include_directories(webview_deps INTERFACE include ${WEBKIT2GTK_INCLUDE_DIRS})
    target_link_libraries(webview_deps INTERFACE ${WEBKIT2GTK_LIBRARIES} Threads::Threads)
    target_compile_definitions(webview_deps INTERFACE WEBVIEW_GTK)
endif()
