if(NOT CLANG_FORMAT_EXE)
    find_program(FOUND_CLANG_FORMAT_EXE "clang-format")
endif()

find_program(DIFF_EXE diff)
find_program(GIT_EXE git)

find_program(SH_EXE sh)
if(WIN32 AND NOT SH_EXE)
    find_program(CMD_EXE cmd REQUIRED)
    # CMD_EXE contains forward slashes which emits an error regarding incorrect syntax, so just keep it simple
    set(SHELL_NAME cmd)
    set(SHELL_CMD "${SHELL_NAME}" /c)
else()
    find_program(SH_EXE sh REQUIRED)
    set(SHELL_NAME sh)
    set(SHELL_CMD "${SHELL_NAME}" -c)
endif()

if(WIN32 AND NOT SH_EXE)
    file(TO_NATIVE_PATH "${FOUND_CLANG_FORMAT_EXE}" FOUND_CLANG_FORMAT_EXE)
    file(TO_NATIVE_PATH "${DIFF_EXE}" DIFF_EXE)
    file(TO_NATIVE_PATH "${GIT_EXE}" GIT_EXE)
else()
    file(TO_CMAKE_PATH "${FOUND_CLANG_FORMAT_EXE}" FOUND_CLANG_FORMAT_EXE)
    file(TO_CMAKE_PATH "${DIFF_EXE}" DIFF_EXE)
    file(TO_CMAKE_PATH "${GIT_EXE}" GIT_EXE)
endif()

# Find source and header files
foreach(DIR ${DIRECTORIES})
    file(TO_CMAKE_PATH "${DIR}" DIR)

    file(GLOB_RECURSE HEADER_FILES RELATIVE "${ROOT_DIR}" "${DIR}/*.h")
    file(GLOB_RECURSE SOURCE_FILES RELATIVE "${ROOT_DIR}" "${DIR}/*.cc")

    list(APPEND DISCOVERED_HEADER_FILES ${HEADER_FILES})
    list(APPEND DISCOVERED_SOURCE_FILES ${SOURCE_FILES})
endforeach()
list(APPEND DISCOVERED_FILES ${DISCOVERED_HEADER_FILES} ${DISCOVERED_SOURCE_FILES})
list(SORT DISCOVERED_HEADER_FILES)
list(SORT DISCOVERED_SOURCE_FILES)
list(SORT DISCOVERED_FILES)

# clang-format
if(FOUND_CLANG_FORMAT_EXE AND (DIFF_EXE OR GIT_EXE))
    foreach(FILE ${DISCOVERED_FILES})
        # Convert some paths to CMake paths just for forward slashes
        file(TO_CMAKE_PATH "${FILE}" FILE_CMAKE)
        if(GIT_EXE)
            set(CMD_LINE_ARGS_RAW "${FOUND_CLANG_FORMAT_EXE}" "${FILE_CMAKE}" | "${GIT_EXE}" diff --no-index --ignore-cr-at-eol -- "${FILE_CMAKE}" -)
        elseif(DIFF_EXE)
            set(CMD_LINE_ARGS_RAW "${FOUND_CLANG_FORMAT_EXE}" "${FILE_CMAKE}" | "${DIFF_EXE}" --unified  "${FILE_CMAKE}" -)
        endif()
        set(CMD_LINE_ARGS)
        foreach(CMD_LINE_ARG_RAW ${CMD_LINE_ARGS_RAW})
            if(SHELL_NAME STREQUAL "sh" AND CMD_LINE_ARG_RAW MATCHES " ")
                set(CMD_LINE_ARG "\"${CMD_LINE_ARG_RAW}\"")
            else()
                set(CMD_LINE_ARG "${CMD_LINE_ARG_RAW}")
            endif()
            list(APPEND CMD_LINE_ARGS "${CMD_LINE_ARG}")
        endforeach()
        if(SHELL_NAME STREQUAL "sh")
            list(JOIN CMD_LINE_ARGS " " CMD_LINE_ARGS)
        endif()
        execute_process(
            COMMAND ${SHELL_CMD} ${CMD_LINE_ARGS}
            WORKING_DIRECTORY "${ROOT_DIR}"
            TIMEOUT 5
            RESULT_VARIABLE DIFF_RESULT
            OUTPUT_VARIABLE DIFF_OUTPUT)
        if(NOT DIFF_RESULT EQUAL 0)
            message(FATAL_ERROR "Code style violation:\n${DIFF_OUTPUT}")
        endif()
    endforeach()
else()
    message(WARNING "clang-format check cannot run due to missing program(s) (need clang-format and either diff or git)")
endif()
