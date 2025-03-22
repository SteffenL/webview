if((NOT UNIX OR APPLE) AND WEBVIEW_WEBKITGTK_API VERSION_EQUAL "6.0")
    return()
endif()

find_package(PkgConfig)

if(NOT PkgConfig_FOUND)
    return()
endif()

pkg_check_modules(GTK gtk4)
pkg_check_modules(WEBKITGTK webkitgtk-6.0)

if(NOT GTK_FOUND OR NOT WEBKITGTK_FOUND)
    return()
endif()

add_executable(webview_example_gtk4)
target_sources(webview_example_gtk4 PRIVATE main.cc)
target_link_directories(webview_example_gtk4 PRIVATE ${GTK_LIBRARY_DIRS} ${WEBKITGTK_LIBRARY_DIRS})
target_link_libraries(webview_example_gtk4 PRIVATE ${GTK_LIBRARIES} ${WEBKITGTK_LIBRARIES} webview::core)
target_include_directories(webview_example_gtk4 PRIVATE ${GTK_INCLUDE_DIRS} ${WEBKITGTK_INCLUDE_DIRS})
target_compile_options(webview_example_gtk4 PRIVATE ${GTK_CFLAGS_OTHER} ${WEBKITGTK_CFLAGS_OTHER})
