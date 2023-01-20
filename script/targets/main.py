from internal.build import FileType, find_c_like_source_files, LanguageStandards, PropertyScopes
from internal.target import RuntimeLinkType, TargetType
from internal.task import Task, TaskPhase, TaskRunner
from internal.toolchain.common import ToolchainId
from internal.workspace import Workspace

import os
import platform
from typing import Iterable


def get_warning_params(toolchain_id: ToolchainId) -> Iterable[str]:
    params = []
    if toolchain_id == ToolchainId.MSVC:
        params += (
            "/W4",
            # warning C4100: '[...]': unreferenced formal parameter
            "/wd4100",
            # warning C4146: unary minus operator applied to unsigned type, result still unsigned
            "/wd4146"
        )
    else:
        params += ("-Wall", "-Wextra", "-pedantic")
        if platform.system() == "Windows":
            # These warnings are emitted because of WebView2 so suppress them.
            params += ("-Wno-unknown-pragmas",
                       "-Wno-unused-parameter", "-Wno-cast-function-type")
    return params


def register(task_runner: TaskRunner, workspace: Workspace):
    system = platform.system()
    source_dir = workspace.get_source_dir()
    options = workspace.get_options()

    toolchain_id = workspace.get_toolchain().get_id()

    # C/C++ standard
    # Visual Studio 2022: C11/C++14 and later are supported.
    # MS WebView2 needs a more recent C++ standard.
    c_standard = LanguageStandards.C11 if toolchain_id == ToolchainId.MSVC else LanguageStandards.C99
    cxx_standard = LanguageStandards.CXX17 if system == "Windows" else LanguageStandards.CXX11

    # Header-only library target
    header_library = workspace.add_target(
        TargetType.INTERFACE, "library_header")
    header_library.add_include_dirs(source_dir)
    header_library.set_language_standard(c_standard)
    header_library.set_language_standard(cxx_standard)
    header_library.set_warning_params(get_warning_params(toolchain_id))
    if system == "Darwin":
        header_library.add_definition("WEBVIEW_COCOA")
        header_library.add_macos_frameworks("WebKit")
    elif system == "Linux":
        header_library.add_definition("WEBVIEW_GTK")
        header_library.add_pkgconfig_libs("gtk+-3.0", "webkit2gtk-4.0")
    elif system == "Windows":
        header_library.add_definition("WEBVIEW_EDGE")
        header_library.add_link_libraries(workspace.get_target("mswebview2"))
        # Add link libraries to help compilers other than MSVC.
        header_library.add_link_libraries(
            "advapi32", "ole32", "shell32", "shlwapi", "user32", "version")
        header_library.set_uses_threads()

    # Shared library target
    shared_library = workspace.add_target(
        TargetType.SHARED_LIBRARY, "library_shared")
    shared_library.set_condition(lambda: options.build_library.get_value())
    shared_library.set_output_name("webview")
    shared_library.add_link_libraries(
        header_library, scope=PropertyScopes.PUBLIC)
    shared_library.add_definition("WEBVIEW_BUILDING")
    shared_library.add_definition(
        "WEBVIEW_SHARED", scope=PropertyScopes.PUBLIC)
    shared_library.add_sources("webview.cc")
    if system == "Windows":
        shared_library.set_runtime_link(RuntimeLinkType.STATIC)

    # Static library target
    static_library = workspace.add_target(
        TargetType.STATIC_LIBRARY, "library_static")
    static_library.set_condition(lambda: options.build_library.get_value())
    static_library.set_output_name("webview_s")
    static_library.add_link_libraries(
        header_library, scope=PropertyScopes.PUBLIC)
    static_library.add_definition("WEBVIEW_BUILDING")
    static_library.add_definition(
        "WEBVIEW_STATIC", scope=PropertyScopes.PUBLIC)
    static_library.add_sources("webview.cc")

    # Example targets
    examples_dir = os.path.join(source_dir, "examples")
    for file_type, source in find_c_like_source_files(examples_dir):
        source_path = os.path.join(examples_dir, source)
        source_file_name = os.path.basename(source)
        example = workspace.add_target(
            TargetType.EXE, "example_{}".format(source_file_name.replace(".", "_")))
        example.set_condition(lambda: options.build_examples.get_value())
        if file_type == FileType.CXX_SOURCE:
            # Use header-only library for C++ examples.
            example.add_link_libraries(header_library)
        elif file_type == FileType.C_SOURCE:
            # Use static library for C examples.
            example.add_link_libraries(static_library)
        example.add_sources(source_path)
        if toolchain_id == ToolchainId.MSVC and file_type == FileType.C_SOURCE:
            # Suppress warnings related to strcat and sprintf.
            example.add_definition("_CRT_SECURE_NO_WARNINGS")

    # Test target
    library_test_program = workspace.add_target(
        TargetType.EXE, "library_test")
    library_test_program.set_condition(
        lambda: options.build_tests.get_value())
    library_test_program.add_link_libraries(header_library)
    library_test_program.add_sources("webview_test.cc")
    library_test = workspace.add_test(
        library_test_program.get_output_file_path(),
        description="Library")
    library_test.set_condition(lambda: options.test.get_value())
