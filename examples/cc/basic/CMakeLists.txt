add_executable(webview_example_basic_cc WIN32)
target_sources(webview_example_basic_cc PRIVATE
    src/main.cc
    $<$<PLATFORM_ID:Windows>:../../shared/win32/resources.rc>)
target_link_libraries(webview_example_basic_cc PRIVATE webview::headers)
