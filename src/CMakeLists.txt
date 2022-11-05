set(TARGET_NAME webview)
set(TARGET_NAME_S ${TARGET_NAME}_s)

add_library(${TARGET_NAME} INTERFACE)
add_library(${TARGET_NAME}::${TARGET_NAME} ALIAS ${TARGET_NAME})

add_library(${TARGET_NAME_S} ${CMAKE_CURRENT_SOURCE_DIR}/webview.cc)
add_library(${TARGET_NAME_S}::${TARGET_NAME_S} ALIAS ${TARGET_NAME_S})

target_compile_features(${TARGET_NAME} INTERFACE cxx_std_11)

target_include_directories(${TARGET_NAME} INTERFACE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_include_directories(${TARGET_NAME_S} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated>
    $<INSTALL_INTERFACE:generated>
)

target_link_libraries(${TARGET_NAME} INTERFACE webview_deps)
target_link_libraries(${TARGET_NAME_S} PUBLIC ${TARGET_NAME} PRIVATE webview_build_support)

if(WIN32 AND NOT MSVC)
    # Specify Windows libraries to link when not using MSVC (possibly MinGW)
    set(WINDOWS_LIBRARIES ole32 shell32 shlwapi user32)
    target_link_libraries(${TARGET_NAME} INTERFACE ${WINDOWS_LIBRARIES})
endif()

set_target_properties(${TARGET_NAME_S} PROPERTIES
    OUTPUT_NAME ${TARGET_NAME}
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

include(GenerateExportHeader)
# Use WEBVIEW_DEPRECATED defined in-source instead of WEBVIEW_DEPRECATED_.
generate_export_header(
    ${TARGET_NAME_S}
    BASE_NAME WEBVIEW
    EXPORT_MACRO_NAME WEBVIEW_API
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/generated/webview/exports.h
    DEPRECATED_MACRO_NAME WEBVIEW_DEPRECATED_
    STATIC_DEFINE WEBVIEW_STATIC
)
