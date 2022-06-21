# To create a new test, create a new file named "NAME_test.cc" where NAME is the name of the test,
# and add a function in the new file with the signature "int NAME(int, char *[])".
# To exclude tests, prefix the name and exclude it below. For example, files named "win32_*"
# should only run on Windows.

set(TEST_DRIVER_NAME webview_tests)
set(TEST_DRIVER_SOURCE ${TEST_DRIVER_NAME}.cc)
file(GLOB ALL_TEST_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS *_test.cc)

# Exclude tests that we do not want to run on this platform.
set(TEST_SOURCES ${ALL_TEST_SOURCES})
foreach(TEST_SOURCE ${ALL_TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    if(NOT WIN32 AND TEST_NAME MATCHES win32_*)
        remove(TEST_SOURCES ${TEST_SOURCE})
    endif()
endforeach()

create_test_sourcelist(TESTS ${TEST_DRIVER_SOURCE} ${TEST_SOURCES})
add_executable(${TEST_DRIVER_NAME} ${TESTS})
target_link_libraries(${TEST_DRIVER_NAME} PRIVATE webview_header webview_build_support)
remove(TESTS ${TEST_DRIVER_SOURCE})

set(TEST_ENV_VARS)
if(WIN32)
    get_target_property(WebView2Loader_IMPORTED_LOCATION WebView2Loader IMPORTED_LOCATION)
    get_filename_component(WebView2Loader_IMPORTED_DIR ${WebView2Loader_IMPORTED_LOCATION} DIRECTORY)
    file(TO_NATIVE_PATH ${WebView2Loader_IMPORTED_DIR} WebView2Loader_IMPORTED_DIR)

    string(REPLACE ";" "\\\\;" TEST_PATHS "$ENV{PATH};${WebView2Loader_IMPORTED_DIR}")
    list(APPEND TEST_ENV_VARS "PATH=${TEST_PATHS}")
endif()
list(LENGTH TEST_ENV_VARS TEST_ENV_VARS_LENGTH)

foreach(TEST ${TESTS})
    get_filename_component(TEST_NAME ${TEST} NAME_WE)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_DRIVER_NAME} ${TEST_NAME})
    if(TEST_ENV_VARS_LENGTH GREATER 0)
        set_tests_properties(${TEST_NAME} PROPERTIES ENVIRONMENT ${TEST_ENV_VARS})
    endif()
endforeach()
