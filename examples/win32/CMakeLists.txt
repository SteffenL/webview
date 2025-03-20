if(NOT WIN32)
    return()
endif()

add_executable(webview_example_win32 WIN32)
target_sources(webview_example_win32 PRIVATE main.cc main.rc main.exe.manifest)
target_link_libraries(webview_example_win32 PRIVATE webview::core)

if(MSVC)
    # Disable generation of manifest file as we have our own file
    target_link_options(webview_example_win32 PRIVATE /MANIFEST:NO)
endif()
