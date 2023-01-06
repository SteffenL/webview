#!/usr/bin/env python3

import sys

min_required_python = (3, 7)
if sys.version_info < min_required_python:
    sys.exit("Python %d.%d or later is required." % min_required_python)
else:
    from cli import create_arg_parser
    from internal.common import Arch
    from internal.cli import parse_options
    from internal.options import format_option_value, Options
    from internal.toolchain.common import ToolchainEnvironmentId
    from internal.toolchain.toolchain import activate_toolchain, detect_toolchain, Toolchain, ToolchainId
    from internal.utility import get_host_arch
    from dataclasses import dataclass
    from enum import Enum
    from glob import glob
    import os
    import platform
    import shutil
    import subprocess
    from typing import Mapping, Sequence


class TargetType(Enum):
    EXECUTABLE = "executable"
    SHARED_LIBRARY = "shared library"
    STATIC_LIBRARY = "static_library"


@dataclass
class Toolchain:
    compile_exe_c: str
    compile_exe_cxx: str
    archive_exe: str
    exe_name_prefix: str
    exe_name_extension: str
    shared_lib_name_prefix: str
    shared_lib_name_extension: str
    static_lib_name_prefix: str
    static_lib_name_extension: str
    obj_name_prefix: str
    obj_name_extension: str


@dataclass
class Workspace:
    source_dir: str
    output_dir: str
    output_bin_dir: str
    output_lib_dir: str
    output_obj_dir: str


@dataclass
class Target:
    type: TargetType
    name: str
    source: str
    standard: str
    output_name: str
    warning_flags: Sequence[str]
    definitions: Sequence[str]
    include_dirs: Sequence[str]
    lib_dirs: Sequence[str]
    libs: Sequence[str]
    extra_compile_flags: Sequence[str]
    extra_link_flags: Sequence[str]


def clean_build_dir(workspace: Workspace):
    dir = workspace.output_dir
    if dir is None or len(dir) == 0 or not os.path.exists(dir):
        return
    print(f"Deleting directory: {dir}...")
    try:
        shutil.rmtree(dir)
    except FileNotFoundError:
        pass


def pre_process_options(options: Options):
    # Enable all checks.
    if options.check.get_value():
        if not options.check_lint.is_explicit():
            options.check_lint.set_value(True)
        if not options.check_style.is_explicit():
            options.check_style.set_value(True)

    # Cannot specify only one of C/C++ compiler.
    if (options.cc.get_value() is None) != (options.cxx.get_value() is None):
        raise Exception(
            "Either both of cc/cxx options or neither must be specified")

    # Set the the host architecture if needed.
    if options.target_arch.get_value() == Arch.NATIVE:
        options.target_arch.set_value(get_host_arch())

    # Building Go examples requires building.
    if options.go_build_examples.get_value() and not options.go_build.is_explicit():
        options.go_build.set_value(True)

    # Running Go tests requires building.
    if options.go_test.get_value() and not options.go_build.is_explicit():
        options.go_build.set_value(True)

    # Building with Go requires dependencies.
    if options.go_build.get_value() and not options.fetch_deps.is_explicit():
        options.fetch_deps.set_value(True)

    # Enable building everything.
    if options.build.get_value():
        if not options.build_library.is_explicit():
            options.build_library.set_value(True)
        if not options.build_examples.is_explicit():
            options.build_examples.set_value(True)
        if not options.build_tests.is_explicit():
            options.build_tests.set_value(True)

    # Running tests requires building tests.
    if options.test.get_value() and not options.build_tests.is_explicit():
        options.build_tests.set_value(True)

    # Building examples requires building library.
    if options.build_examples.get_value() and not options.build_library.is_explicit():
        options.build_library.set_value(True)

    # Building tests requires building library.
    if options.build_tests.get_value() and not options.build_library.is_explicit():
        options.build_library.set_value(True)

    # Building library requires dependencies.
    if options.build_library.get_value() and not options.fetch_deps.is_explicit():
        options.fetch_deps.set_value(True)

    # Set the C compiler binary based on the environment.
    if not options.cc.is_explicit():
        options.cc.set_value(os.environ.get("CC", None))

    # Set the C++ compiler binary based on the environment.
    if not options.cxx.is_explicit():
        options.cxx.set_value(os.environ.get("CXX", None))

    return options


def gcc_compile(target: Target, workspace: Workspace, toolchain: Toolchain):
    print(f"Building target {target.name}...")
    target_output_name = target.name if target.output_name is None else target.output_name

    # Compile
    obj_file_path = os.path.join(
        workspace.output_obj_dir,
        target.name,
        "".join((
            target.source,
            toolchain.obj_name_extension
        )))
    source_ext = os.path.splitext(target.source)[1]
    compile_exe = toolchain.compile_exe_cxx if source_ext in (
        ".cc", ".cpp", ".cxx") else toolchain.compile_exe_c if source_ext == ".c" else toolchain.compile_exe_cxx
    compile_command = [
        compile_exe,
        target.source,
        *target.warning_flags,
        "-c",
        "-std=" + target.standard,
        "-o",
        obj_file_path,
        *[f"-D{s}" for s in target.definitions],
        *[f"-I{s}" for s in target.include_dirs],
        *target.extra_compile_flags
    ]
    if target.type == TargetType.SHARED_LIBRARY:
        compile_command.append("-fPIC")
    os.makedirs(os.path.dirname(obj_file_path), exist_ok=True)
    subprocess.check_call(compile_command, cwd=workspace.source_dir)

    # Archive
    if target.type == TargetType.STATIC_LIBRARY:
        lib_file_path = os.path.join(
            workspace.output_lib_dir, "".join((
                toolchain.static_lib_name_prefix,
                target_output_name,
                toolchain.static_lib_name_extension)))
        os.makedirs(os.path.dirname(lib_file_path), exist_ok=True)
        command = [
            toolchain.archive_exe,
            "rcs",
            lib_file_path,
            obj_file_path
        ]
        subprocess.check_call(command, cwd=workspace.source_dir)
        return

    # Link
    if target.type in (TargetType.EXECUTABLE, TargetType.SHARED_LIBRARY):
        name_prefix = {TargetType.EXECUTABLE: toolchain.exe_name_prefix,
                       TargetType.SHARED_LIBRARY: toolchain.shared_lib_name_prefix}[target.type]
        name_extension = {TargetType.EXECUTABLE: toolchain.exe_name_extension,
                          TargetType.SHARED_LIBRARY: toolchain.shared_lib_name_extension}[target.type]
        bin_file_path = os.path.join(
            workspace.output_bin_dir, "".join((
                name_prefix,
                target_output_name,
                name_extension)))
        os.makedirs(os.path.dirname(bin_file_path), exist_ok=True)
        link_command = [
            compile_exe,
            obj_file_path,
            *target.warning_flags,
            "-o",
            bin_file_path,
            *[f"-L{s}" for s in target.lib_dirs],
            *[f"-l{s}" for s in target.libs],
            *target.extra_link_flags
        ]
        if target.type == TargetType.SHARED_LIBRARY:
            link_command.append("-shared")
        subprocess.check_call(link_command, cwd=workspace.source_dir)
        return


def main(args):
    system = platform.system()
    options = pre_process_options(parse_options(args, create_arg_parser))
    architecture = options.target_arch.get_value()
    script_dir = os.path.dirname(os.path.normpath(__file__))
    source_dir = os.path.dirname(script_dir)
    build_root_dir = os.path.join(source_dir, "build")
    build_dir = os.path.join(build_root_dir, architecture.value.lower())
    build_bin_dir = os.path.join(build_dir, "bin")
    build_lib_dir = os.path.join(build_dir, "lib")
    build_obj_dir = os.path.join(build_dir, "obj")

    """toolchain_env = options.load_toolchain.get_value()
    if toolchain_env is not None:
        activate_toolchain(toolchain_env, architecture)

    toolchain = detect_toolchain(architecture=architecture,
                                 cc_override=options.cc.get_value(),
                                 cxx_override=options.cxx.get_value())
    toolchain_id = toolchain.get_id()"""

    compile_flags: Mapping[str, Sequence[str]] = {"c": [], "cc": []}
    link_flags: Mapping[str, Sequence[str]] = {"c": [], "cc": []}
    include_dirs: Mapping[str, Sequence[str]] = {"c": [], "cc": []}
    standard: Mapping[str, str] = {"c": "c99", "cc": "c++11"}

    include_dirs["cc"].append(source_dir)

    gcc_warning_flags = ["-Wall", "-Wextra", "-pedantic"]

    # for file in glob("*.c")

    # sources = ("webview.cc",
    #           *glob("examples/*.cc", root_dir=source_dir),
    #           *glob("examples/*.c", root_dir=source_dir))
    # print(sources)

    pkgconfig_libs = ["gtk+-3.0", "webkit2gtk-4.0"]
    pkgconfig_cflags = subprocess.check_output(["pkg-config", "--cflags", *pkgconfig_libs],
                                               encoding="utf8").strip().split(" ")
    pkgconfig_ldflags = subprocess.check_output(["pkg-config", "--libs", *pkgconfig_libs],
                                                encoding="utf8").strip().split(" ")
    gcc_standard_c = "c99"
    gcc_standard_cxx = "c++11"

    toolchain = Toolchain(
        compile_exe_c="cc",
        compile_exe_cxx="c++",
        archive_exe="ar",
        exe_name_prefix="",
        exe_name_extension="",
        shared_lib_name_prefix="lib",
        shared_lib_name_extension=".so",
        static_lib_name_prefix="lib",
        static_lib_name_extension=".a",
        obj_name_prefix="",
        obj_name_extension=".o"
    )

    workspace = Workspace(
        source_dir=source_dir,
        output_dir=build_dir,
        output_bin_dir=build_bin_dir,
        output_lib_dir=build_lib_dir,
        output_obj_dir=build_obj_dir
    )

    targets = []

    if options.build_library.get_value():
        targets += (
            Target(
                type=TargetType.SHARED_LIBRARY,
                name="library_shared",
                source="webview.cc",
                standard=gcc_standard_cxx,
                output_name="webview",
                warning_flags=gcc_warning_flags,
                definitions=["WEBVIEW_BUILDING", "WEBVIEW_SHARED"],
                include_dirs=[],
                lib_dirs=[],
                libs=[],
                extra_compile_flags=pkgconfig_cflags,
                extra_link_flags=pkgconfig_ldflags
            ),
            Target(
                type=TargetType.STATIC_LIBRARY,
                name="library_static",
                source="webview.cc",
                standard=gcc_standard_cxx,
                output_name="webview_s",
                warning_flags=gcc_warning_flags,
                definitions=["WEBVIEW_BUILDING", "WEBVIEW_STATIC"],
                include_dirs=[],
                lib_dirs=[],
                libs=[],
                extra_compile_flags=pkgconfig_cflags,
                extra_link_flags=[]
            )
        )

    if options.build_examples.get_value():
        targets += [Target(
            type=TargetType.EXECUTABLE,
            name="example_" + os.path.basename(source).replace(".", "_"),
            source=source,
            standard=gcc_standard_cxx,
            output_name=None,
            warning_flags=gcc_warning_flags,
            definitions=[],
            include_dirs=[source_dir],
            lib_dirs=[],
            libs=[],
            extra_compile_flags=pkgconfig_cflags,
            extra_link_flags=pkgconfig_ldflags
        ) for source in glob("examples/*.cc", root_dir=source_dir)]
        targets += [Target(
            type=TargetType.EXECUTABLE,
            name="example_" + os.path.basename(source).replace(".", "_"),
            source=source,
            standard=gcc_standard_c,
            output_name=None,
            warning_flags=gcc_warning_flags,
            definitions=[],
            include_dirs=[source_dir],
            lib_dirs=[build_lib_dir],
            libs=["webview_s", "stdc++"],
            extra_compile_flags=pkgconfig_cflags,
            extra_link_flags=pkgconfig_ldflags
        ) for source in glob("examples/*.c", root_dir=source_dir)]

    if options.build_tests.get_value():
        targets.append(Target(
            type=TargetType.EXECUTABLE,
            name="library_test",
            source="webview_test.cc",
            standard=gcc_standard_cxx,
            output_name="webview_test",
            warning_flags=gcc_warning_flags,
            definitions=[],
            include_dirs=[source_dir],
            lib_dirs=[],
            libs=[],
            extra_compile_flags=pkgconfig_cflags,
            extra_link_flags=pkgconfig_ldflags
        ))

    if options.clean.get_value():
        clean_build_dir(workspace)

    for target in targets:
        gcc_compile(target, workspace, toolchain)

    if options.go_build_examples.get_value():
        go_examples = [(source, os.path.join(build_bin_dir, os.path.basename(source).replace(
            ".", "_"))) for source in glob("examples/*.go", root_dir=source_dir)]
        for source, output_path in go_examples:
            print(f"Building Go example {source}...")
            subprocess.check_call(
                ("go", "build", "-o", output_path, source), cwd=source_dir)

    if options.go_test.get_value():
        print(f"Running Go test...")
        subprocess.check_call(("go", "test", "-v"), cwd=source_dir)


if __name__ == "__main__":
    main(sys.argv[1:])
