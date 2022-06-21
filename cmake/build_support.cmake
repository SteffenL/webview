if(MSVC)
    set(COMPILE_OPTIONS
        /utf-8
        /W4
        /wd4100 # unused parameters
    )
else()
    set(COMPILE_OPTIONS
        -Wall
        -Wextra
        -pedantic
        -Wno-unused-parameter
    )
    if(WIN32)
        list(APPEND COMPILE_OPTIONS -Wno-cast-function-type)
    endif()
endif()
add_library(webview_build_support INTERFACE)
target_compile_options(webview_build_support INTERFACE ${COMPILE_OPTIONS})
