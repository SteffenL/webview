file(GLOB_RECURSE EXAMPLE_SOURCES CONFIGURE_DEPENDS *.c *.cc)

foreach(EXAMPLE_SOURCE ${EXAMPLE_SOURCES})
    cmake_path(GET EXAMPLE_SOURCE FILENAME TARGET_NAME)
    string(REPLACE "." "_" TARGET_NAME ${TARGET_NAME})
    add_executable(${TARGET_NAME} WIN32 MACOSX_BUNDLE ${EXAMPLE_SOURCE})
    target_link_libraries(${TARGET_NAME} PRIVATE webview_build_support webview_s)
endforeach()
