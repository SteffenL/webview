file(GLOB_RECURSE EXAMPLE_SOURCES CONFIGURE_DEPENDS *.c *.cc)

foreach(EXAMPLE_SOURCE ${EXAMPLE_SOURCES})
    cmake_path(GET EXAMPLE_SOURCE FILENAME EXAMPLE_NAME)
    string(REPLACE "." "_" EXAMPLE_NAME ${EXAMPLE_NAME})
    set(TARGET_NAME webview_example_${EXAMPLE_NAME})
    add_executable(${TARGET_NAME} WIN32 MACOSX_BUNDLE ${EXAMPLE_SOURCE})
    target_link_libraries(${TARGET_NAME} PRIVATE webview_build_support webview_core_static)
    set_target_properties(${TARGET_NAME} PROPERTIES
        OUTPUT_NAME example_${EXAMPLE_NAME}
    )
endforeach()
