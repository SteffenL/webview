file(GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS *.c)

foreach(SOURCE ${SOURCES})
    get_filename_component(NAME ${SOURCE} NAME_WE)
    add_executable(examples_c_${NAME} WIN32 MACOSX_BUNDLE ${SOURCE})
    set_target_properties(examples_c_${NAME} PROPERTIES OUTPUT_NAME ${NAME})
    target_link_libraries(examples_c_${NAME} PRIVATE webview_static webview_build_support)
    target_compile_definitions(examples_c_${NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
endforeach()
