add_library(webview_deps INTERFACE)

if(WIN32)
    if(NOT TARGET WebView2 AND WEBVIEW_FETCH_DEPS)
        include(${CMAKE_CURRENT_LIST_DIR}/webview2.cmake)
        fetch_webview2(${WEBVIEW_MSWEBVIEW2_VERSION})
    endif()
    target_link_libraries(webview_deps INTERFACE WebView2)
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
