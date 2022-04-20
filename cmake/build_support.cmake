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
        -Wall -Wextra -Wconversion -Wsign-conversion -pedantic
        -Wno-unused-parameter
    )
endif()
set(TARGET_NAME webview_build_support)
add_library(${TARGET_NAME} INTERFACE)
target_compile_options(${TARGET_NAME} INTERFACE ${COMPILE_OPTIONS})
