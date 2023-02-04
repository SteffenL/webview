set(PRIVATE_LIBRARIES)
# Windows libraries to link when not using MSVC (possibly MinGW)
if(WIN32 AND NOT MSVC)
    list(APPEND PRIVATE_LIBRARIES advapi32 ole32 shell32 shlwapi user32 version)
endif()

#
# Base for the library targets
#

set(TARGET_NAME_BASE webview_core_base)
add_library(${TARGET_NAME_BASE} INTERFACE)
add_library(${PROJECT_NAME}::core_base ALIAS ${TARGET_NAME_BASE})
target_compile_features(${TARGET_NAME_BASE} INTERFACE cxx_std_11)
target_compile_definitions(${TARGET_NAME_BASE} INTERFACE
    WEBVIEW_MSWEBVIEW2_BUILTIN_IMPL=$<BOOL:${WEBVIEW_MSWEBVIEW2_BUILTIN_IMPL}>
    WEBVIEW_MSWEBVIEW2_EXPLICIT_LINK=$<BOOL:${WEBVIEW_MSWEBVIEW2_EXPLICIT_LINK}>
)
target_include_directories(${TARGET_NAME_BASE} INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Third party libraries
if(WIN32)
    find_package(WebView2 ${WEBVIEW_MSWEBVIEW2_VERSION} REQUIRED)
    set(MSWEBVIEW2_LIBRARY WebView2::sdk)
    if(NOT WEBVIEW_MSWEBVIEW2_BUILTIN_IMPL)
        if(WEBVIEW_MSWEBVIEW2_USE_STATIC_LIBRARY)
            set(MSWEBVIEW2_LIBRARY WebView2::loader_s)
        elseif(NOT WEBVIEW_MSWEBVIEW2_EXPLICIT_LINK)
            set(MSWEBVIEW2_LIBRARY WebView2::loader)
        endif()
    endif()
    target_link_libraries(${TARGET_NAME_BASE} INTERFACE ${MSWEBVIEW2_LIBRARY})
    target_compile_definitions(${TARGET_NAME_BASE} INTERFACE WEBVIEW_EDGE)
    # WebView2 requires a more recent C++ standard.
    target_compile_features(${TARGET_NAME_BASE} INTERFACE cxx_std_17)
elseif(APPLE)
    find_library(WEBKIT_LIBRARY WebKit REQUIRED)
    target_link_libraries(${TARGET_NAME_BASE} INTERFACE ${WEBKIT_LIBRARY})
    target_compile_definitions(${TARGET_NAME_BASE} INTERFACE WEBVIEW_COCOA)
elseif(UNIX)
    find_package(PkgConfig REQUIRED)
    find_package(Threads REQUIRED)
    pkg_check_modules(GTK REQUIRED gtk+-3.0)
    pkg_check_modules(WEBKIT2GTK REQUIRED webkit2gtk-4.0)
    target_include_directories(${TARGET_NAME_BASE} INTERFACE include ${WEBKIT2GTK_INCLUDE_DIRS})
    target_link_libraries(${TARGET_NAME_BASE} INTERFACE ${WEBKIT2GTK_LIBRARIES} Threads::Threads)
    target_compile_definitions(${TARGET_NAME_BASE} INTERFACE WEBVIEW_GTK)
endif()


#
# Header-only library
#

set(TARGET_NAME_HEADER webview_core_header)
add_library(${TARGET_NAME_HEADER} INTERFACE)
add_library(${PROJECT_NAME}::core_header ALIAS ${TARGET_NAME_HEADER})
target_compile_features(${TARGET_NAME_HEADER} INTERFACE cxx_std_11)
# TODO: When WEBVIEW_API defaults to "inline" in the source code then we can remove it here.
target_compile_definitions(${TARGET_NAME_HEADER} INTERFACE WEBVIEW_API=inline)
target_link_libraries(${TARGET_NAME_HEADER} INTERFACE ${TARGET_NAME_BASE} ${PRIVATE_LIBRARIES})

#
# Shared library
#

set(TARGET_NAME_SHARED webview_core_shared)
add_library(${TARGET_NAME_SHARED} SHARED src/webview.cc)
add_library(${PROJECT_NAME}::core_shared ALIAS ${TARGET_NAME_SHARED})
target_compile_definitions(${TARGET_NAME_SHARED} INTERFACE WEBVIEW_HEADER WEBVIEW_SHARED PRIVATE WEBVIEW_BUILD_SHARED)
target_link_libraries(${TARGET_NAME_SHARED} PUBLIC ${TARGET_NAME_BASE} PRIVATE webview_build_support ${PRIVATE_LIBRARIES})
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

#
# Static library
#

set(TARGET_NAME_STATIC webview_core_static)
add_library(${TARGET_NAME_STATIC} STATIC src/webview.cc)
add_library(${PROJECT_NAME}::core_static ALIAS ${TARGET_NAME_STATIC})
target_compile_definitions(${TARGET_NAME_STATIC} INTERFACE WEBVIEW_HEADER PUBLIC WEBVIEW_STATIC)
target_link_libraries(${TARGET_NAME_STATIC} PUBLIC ${TARGET_NAME_BASE} ${PRIVATE_LIBRARIES} PRIVATE webview_build_support)
set_target_properties(${TARGET_NAME_STATIC} PROPERTIES OUTPUT_NAME webview_s)

#
# Common
#

set_target_properties(${TARGET_NAME_SHARED} ${TARGET_NAME_STATIC} PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)
