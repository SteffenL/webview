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
add_library(webview_build_support INTERFACE)
target_compile_options(webview_build_support INTERFACE ${COMPILE_OPTIONS})
if(MSVC)
    target_compile_definitions(webview_build_support INTERFACE _CRT_SECURE_NO_WARNINGS)
endif()
