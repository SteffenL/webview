file(GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS *.cc)

foreach(SOURCE ${SOURCES})
    get_filename_component(NAME ${SOURCE} NAME_WE)
    add_executable(examples_cc_${NAME} WIN32 MACOSX_BUNDLE ${SOURCE})
    set_target_properties(examples_cc_${NAME} PROPERTIES OUTPUT_NAME ${NAME})
    target_link_libraries(examples_cc_${NAME} PRIVATE webview_header webview_build_support)
endforeach()
