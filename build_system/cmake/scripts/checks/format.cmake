if(CLANG_FORMAT_EXE)
    find_program(FOUND_CLANG_FORMAT_EXE "${CLANG_FORMAT_EXE}")
else()
    find_program(FOUND_CLANG_FORMAT_EXE "clang-format")
endif()

find_program(DIFF_EXE diff)
find_program(GIT_EXE git)

if(NOT FOUND_CLANG_FORMAT_EXE OR NOT (DIFF_EXE OR GIT_EXE))
    message(WARNING "clang-format check cannot run due to missing program(s) (need clang-format and either diff or git)")
endif()

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

file(GLOB_RECURSE DISCOVERED_FILES RELATIVE "${ROOT_DIR}" "${ROOT_DIR}/*.h" "${ROOT_DIR}/*.cc")

file(TO_CMAKE_PATH "${ROOT_DIR}" ROOT_DIR_CMAKE)
file(REAL_PATH "${ROOT_DIR_CMAKE}" ROOT_DIR_CMAKE)

# Normalize paths of exclude directories
set(EXCLUDE_DIRS_TEMP ${EXCLUDE_DIRS})
set(EXCLUDE_DIRS)
foreach(EXCLUDE_DIR ${EXCLUDE_DIRS_TEMP})
    file(TO_CMAKE_PATH "${EXCLUDE_DIR}" EXCLUDE_DIR_CMAKE)
    file(REAL_PATH "${EXCLUDE_DIR_CMAKE}" EXCLUDE_DIR_CMAKE)
    list(APPEND EXCLUDE_DIRS "${EXCLUDE_DIR_CMAKE}")
endforeach()

foreach(FILE ${DISCOVERED_FILES})
    # Convert some paths to CMake paths to use forward slashes
    file(TO_CMAKE_PATH "${FILE}" FILE_CMAKE)
    # Absolute path of file
    set(FILE_ABS_CMAKE "${ROOT_DIR_CMAKE}/${FILE_CMAKE}")

    # Skip based on exclude directories
    set(SKIP FALSE)
    foreach(EXCLUDE_DIR ${EXCLUDE_DIRS})
        file(TO_CMAKE_PATH "${EXCLUDE_DIR}" EXCLUDE_DIR_CMAKE)
        file(REAL_PATH "${EXCLUDE_DIR_CMAKE}" EXCLUDE_DIR_CMAKE)

        # Find exclude directory at the beginning of the file path
        string(FIND "${FILE_ABS_CMAKE}" "${EXCLUDE_DIR}/" EXCLUDE_DIR_POSITION)
        if(EXCLUDE_DIR_POSITION EQUAL 0)
            set(SKIP TRUE)
            break()
        endif()
    endforeach()
    if(SKIP)
        continue()
    endif()

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
        RESULT_VARIABLE DIFF_RESULT
        OUTPUT_VARIABLE DIFF_OUTPUT)

    if(NOT DIFF_RESULT EQUAL 0)
        message(FATAL_ERROR "Code style violation:\n${DIFF_OUTPUT}")
    endif()
endforeach()
