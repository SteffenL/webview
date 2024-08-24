add_executable(webview_example_basic_c WIN32)
target_sources(webview_example_basic_c PRIVATE
    src/main.c
    $<$<PLATFORM_ID:Windows>:../../shared/win32/resources.rc>)
target_link_libraries(webview_example_basic_c PRIVATE webview::static)

if(MSVC)
    # Disable some warnings when using MSVC
    target_compile_definitions(webview_example_basic_c PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
