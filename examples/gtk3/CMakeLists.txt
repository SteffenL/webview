if((NOT UNIX OR APPLE) AND WEBVIEW_WEBKITGTK_API VERSION_GREATER "4.1")
    return()
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED gtk+-3.0)
pkg_check_modules(WEBKITGTK REQUIRED webkit2gtk-4.1)

add_executable(webview_example_gtk3)
target_sources(webview_example_gtk3 PRIVATE main.cc)
target_link_directories(webview_example_gtk3 PRIVATE ${GTK_LIBRARY_DIRS} ${WEBKITGTK_LIBRARY_DIRS})
target_link_libraries(webview_example_gtk3 PRIVATE ${GTK_LIBRARIES} ${WEBKITGTK_LIBRARIES} webview::core)
target_include_directories(webview_example_gtk3 PRIVATE ${GTK_INCLUDE_DIRS} ${WEBKITGTK_INCLUDE_DIRS})
target_compile_options(webview_example_gtk3 PRIVATE ${GTK_CFLAGS_OTHER} ${WEBKITGTK_CFLAGS_OTHER})
