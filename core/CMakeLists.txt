#
# Header-only library
#

set(TARGET_NAME_HEADER webview_core_header)
add_library(${TARGET_NAME_HEADER} INTERFACE)
add_library(${TARGET_NAME_HEADER}::${TARGET_NAME_HEADER} ALIAS ${TARGET_NAME_HEADER})
target_compile_features(${TARGET_NAME_HEADER} INTERFACE cxx_std_11)
target_compile_definitions(${TARGET_NAME_HEADER} INTERFACE WEBVIEW_MSWEBVIEW2_BUILTIN_IMPL=$<BOOL:WEBVIEW_MSWEBVIEW2_BUILTIN_IMPL>)
target_include_directories(${TARGET_NAME_HEADER} INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(${TARGET_NAME_HEADER} INTERFACE webview_deps)
if(WIN32 AND NOT MSVC)
    # Specify Windows libraries to link when not using MSVC (possibly MinGW)
    set(WINDOWS_LIBRARIES advapi32 ole32 shell32 shlwapi user32 version)
    target_link_libraries(${TARGET_NAME_HEADER} INTERFACE ${WINDOWS_LIBRARIES})
endif()

#
# Shared library
#

set(TARGET_NAME_SHARED webview_core_shared)
add_library(${TARGET_NAME_SHARED} SHARED src/webview.cc)
add_library(${TARGET_NAME_SHARED}::${TARGET_NAME_SHARED} ALIAS ${TARGET_NAME_SHARED})
#target_compile_definitions(${TARGET_NAME_STATIC} PUBLIC WEBVIEW_SHARED PRIVATE WEBVIEW_BUILDING)
target_include_directories(${TARGET_NAME_SHARED} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated>
    $<INSTALL_INTERFACE:generated>
)
target_link_libraries(${TARGET_NAME_SHARED} PUBLIC ${TARGET_NAME_HEADER} PRIVATE webview_build_support)
set_target_properties(${TARGET_NAME_SHARED} PROPERTIES
    OUTPUT_NAME webview
    SOVERSION ${PROJECT_VERSION_MAJOR}
    VERSION ${PROJECT_VERSION}
)
# RPATH
if(NOT APPLE)
    set_target_properties(${TARGET_NAME_SHARED} PROPERTIES
        INSTALL_RPATH "$ORIGIN;$ORIGIN/../lib"
    )
endif()
# Use WEBVIEW_DEPRECATED defined in-source instead of WEBVIEW_DEPRECATED_.
generate_export_header(
    ${TARGET_NAME_SHARED}
    BASE_NAME WEBVIEW
    EXPORT_MACRO_NAME WEBVIEW_API
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/generated/webview/exports.h
    DEPRECATED_MACRO_NAME WEBVIEW_DEPRECATED_
    STATIC_DEFINE WEBVIEW_STATIC
)

#
# Static library
#

set(TARGET_NAME_STATIC webview_core_static)
add_library(${TARGET_NAME_STATIC} STATIC src/webview.cc)
add_library(${TARGET_NAME_STATIC}::${TARGET_NAME_STATIC} ALIAS ${TARGET_NAME_STATIC})
#target_compile_definitions(${TARGET_NAME_STATIC} PUBLIC WEBVIEW_STATIC)
target_link_libraries(${TARGET_NAME_STATIC} PUBLIC ${TARGET_NAME_HEADER} PRIVATE webview_build_support)
set_target_properties(${TARGET_NAME_STATIC} PROPERTIES
    OUTPUT_NAME webview_s
)

#
# Common
#

set_target_properties(${TARGET_NAME_SHARED} ${TARGET_NAME_STATIC} PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)
install(TARGETS ${TARGET_NAME_SHARED} ${TARGET_NAME_STATIC}
    RUNTIME 
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT webview_runtime
    LIBRARY
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT webview_runtime
        NAMELINK_COMPONENT webview_development
    ARCHIVE
        DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT webview_development
)
