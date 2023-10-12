if(CLANG_TIDY_EXE)
    find_program(FOUND_CLANG_TIDY_EXE "${CLANG_TIDY_EXE}")
else()
    find_program(FOUND_CLANG_TIDY_EXE "clang-tidy")
endif()

# Use run-clang-tidy which runs clang-tidy over the files in the compilation database (compile_commands.json)
if(FOUND_CLANG_TIDY_EXE)
    get_filename_component(CLANG_TIDY_EXE_DIR "${FOUND_CLANG_TIDY_EXE}" DIRECTORY)
    get_filename_component(CLANG_TIDY_EXE_NAME "${FOUND_CLANG_TIDY_EXE}" NAME)
    set(RUN_CLANG_TIDY_EXE "${CLANG_TIDY_EXE_DIR}/run-${CLANG_TIDY_EXE_NAME}")
endif()

if(NOT RUN_CLANG_TIDY_EXE)
    message(WARNING "clang-tidy check cannot run due to missing program(s) (need run-clang-tidy)")
endif()

execute_process(
    COMMAND "${RUN_CLANG_TIDY_EXE}" -p "${BUILD_DIR}" -clang-tidy-binary "${FOUND_CLANG_TIDY_EXE}"
    WORKING_DIRECTORY "${ROOT_DIR}"
    RESULT_VARIABLE TIDY_RESULT)
if(NOT TIDY_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to check file(s) with clang-tidy")
endif()
