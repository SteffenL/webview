if(MSVC)
    set(COMPILE_OPTIONS
        /utf-8
        /WX
        /W4
        /wd4100 # unused parameters
    )
else()
    set(COMPILE_OPTIONS
        -Werror
        -Wall -Wextra -pedantic
        -Wno-unused-parameter
    )
    # Likely MinGW
    if(WIN32)
        list(APPEND COMPILE_OPTIONS
            -Wno-cast-function-type # cast between incompatible function types [...] - required for GetProcAddress
        )
    endif()
endif()

set(TARGET_NAME webview_build_support)

add_library(${TARGET_NAME} INTERFACE)
add_library(${PROJECT_NAME}::build_support ALIAS ${TARGET_NAME})

target_compile_options(${TARGET_NAME} INTERFACE ${COMPILE_OPTIONS})

if(MSVC)
    target_compile_definitions(${TARGET_NAME} INTERFACE _CRT_SECURE_NO_WARNINGS)
endif()

set_target_properties(${TARGET_NAME} PROPERTIES EXPORT_NAME build_support)
