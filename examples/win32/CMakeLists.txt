if(NOT WIN32)
    return()
endif()

add_executable(webview_example_win32 WIN32)
target_sources(webview_example_win32 PRIVATE main.cc main.rc main.exe.manifest)
target_link_libraries(webview_example_win32 PRIVATE webview::core)
