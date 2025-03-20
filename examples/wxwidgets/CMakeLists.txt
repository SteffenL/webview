find_package(wxWidgets 3)

if(NOT wxWidgets_FOUND)
    return()
endif()

add_executable(webview_example_wxwidgets)
target_sources(webview_example_wxwidgets PRIVATE main.cc)
target_link_libraries(webview_example_wxwidgets PRIVATE wxWidgets::wxWidgets webview::core)
