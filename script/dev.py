#!/usr/bin/env python3

from glob import glob
import os
import shutil
import subprocess
import sys
from typing import Mapping
from common import *

root_dir = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
build_dir = os.path.join(root_dir, "build")
source_dir = root_dir

def are_examples_enabled(cmake_cache: Mapping[str, str]):
    """Returns whether examples are enabled in the CMake cache."""
    return cmake_cache["BUILD_EXAMPLES"] == "ON"

def is_build_configured(build_dir: str):
    """Returns whether the specified build directory has been configured, i.e. has a CMake cache."""
    return has_cmake_cache(build_dir)

def is_testing_enabled(cmake_cache: Mapping[str, str]):
    """Returns whether tests are enabled in the CMake cache."""
    return cmake_cache["ENABLE_TESTS"] == "ON"

def check_build_configured(build_dir: str):
    """Checks if the build has been configured, and trminates the program if otherwise."""
    if not is_build_configured(build_dir):
        info("Please configure the build.", file=sys.stderr)
        sys.exit(1);

def check_testing_enabled(cmake_cache: Mapping[str, str]):
    """Checks if the build was configured with testing enabled, and trminates the program if otherwise."""
    if not is_testing_enabled(cmake_cache):
        info("Testing is not enabled.", file=sys.stderr)
        sys.exit(1);

def get_dev_exec_dirs(cmake_cache: Mapping[str, str], arch: str, append_to_existing: bool = False):
    """Returns a sequence of directories in which to search for executables."""
    result = []
    if append_to_existing:
        result += os.get_exec_path()
    if is_windows():
        result.append(os.path.realpath(os.path.join(cmake_cache["microsoft_web_webview2_SOURCE_DIR"], "build", "native", arch)))
    return result

def get_dev_shell_comands(cmake_cache: Mapping[str, str], arch: str):
    """Returns a sequence of shell commands for setting up the development environment in a shell."""
    result = []
    current_shell_id = detect_current_shell()
    for exec_dir in get_dev_exec_dirs(cmake_cache, arch):
        result.append(make_shell_export_var_cmd("PATH", exec_dir, "PATH", shell=current_shell_id))
    return result

def get_dev_env_vars(cmake_cache: Mapping[str, str], arch: str = "x64"):
    """Returns a dictionary of environment variables that aid development of this project."""
    return dict((
        ("PATH", join_paths(get_dev_exec_dirs(cmake_cache, arch, True))),
    ))

class BuildType(Enum):
    DEBUG = "Debug"
    RELEASE = "Release"

class CompilerId(Enum):
    GCC = "gcc"
    CLANG = "clang"
    MSVC = "msvc"

def cmd_build():
    """Builds the library using CMake, and if configured, examples and tests."""
    check_build_configured(build_dir)
    cmake_cache = load_cmake_cache_from_dir(build_dir)
    build_type = cmake_cache.get("CMAKE_BUILD_TYPE")
    info("Building...")
    cmd = ["cmake", "--build", build_dir]
    if build_type:
        cmd += ("--config", build_type)
    subprocess.check_call(cmd)

def cmd_clean():
    """Deletes the build directory."""
    info("Cleaning...")
    if build_dir and os.path.exists(build_dir):
        shutil.rmtree(build_dir)

def cmd_configure(build_type: BuildType = BuildType.RELEASE, examples: bool = False, tests: bool = False, webview2_version: str = None, webview2_shared: bool = False):
    """
    Configures the build using CMake.

    Arguments:
      - build_type       -- Build type.
      - examples         -- Enable examples.
      - tests            -- Enable tests.
      - webview2_version -- WebView2 version to use.
      - webview2_shared  -- Use WebView2 as a shared library.
    """
    info("Configuring...")
    cmd = ["cmake"]
    if shutil.which("ninja"):
        cmd += ("-G", "Ninja")
    cmd += (
        "-B", build_dir,
        "-S", source_dir,
        "-DBUILD_EXAMPLES=" + ("ON" if examples else "OFF"),
        "-DENABLE_TESTS=" + ("ON" if tests else "OFF"),
        "-DWEBVIEW2_SHARED=" + ("ON" if webview2_shared else "OFF")
    )
    if webview2_version:
        cmd += (f"-DWEBVIEW2_VERSION={webview2_version}",)
    if build_type:
        cmd += (f"-DCMAKE_BUILD_TYPE={build_type.value}",)
    subprocess.check_call(cmd)

def cmd_devenv(compiler_id: CompilerId = None, arch: str = "x64", shell: ShellId = None):
    """
    Prints command lines that can be used to aid development for the specified shell.

    Arguments:
      - compiler_id -- Compiler ID.
      - arch        -- Target architecture.
      - shell       -- Output format of commands.
    """
    check_build_configured(build_dir)
    cmake_cache = load_cmake_cache_from_dir(build_dir)

    search_paths = ("cmake", "ninja")
    search_paths = [os.path.dirname(shutil.which(s)) for s in search_paths]

    if is_windows():
        webview2_shared_lib_path = os.path.realpath(os.path.join(cmake_cache["microsoft_web_webview2_SOURCE_DIR"], "build", "native", arch))
        search_paths.append(webview2_shared_lib_path)

    shell = ShellId(shell) if shell else detect_current_shell()
    info(make_shell_export_var_cmd("PATH", os.pathsep.join(search_paths), append_to="PATH", shell=shell))

def cmd_go_build():
    """Builds Go examples using "go build"."""
    check_build_configured(build_dir)
    info("Go: Building...")
    subprocess.check_call(("go", "build"))
    if are_examples_enabled(load_cmake_cache_from_dir(build_dir)):
        go_source_examples_dir = os.path.join(source_dir, "examples", "go")
        go_build_examples_dir = os.path.join(build_dir, "examples", "go")
        glob_pattern = go_source_examples_dir.replace("\\", "/") + "/**/*.go"
        go_files = glob(glob_pattern, recursive=True)
        for go_file in go_files:
            go_file = os.path.realpath(go_file)
            info(f"Go: Building \"{go_file}\"...")
            go_file_in_path = os.path.join(go_source_examples_dir, go_file)
            go_file_out_path = os.path.join(go_build_examples_dir, os.path.splitext(go_file)[0]) + (".exe" if is_windows() else "")
            cmd = ["go", "build"]
            cmd += ("-ldflags=-H windowsgui",) if is_windows() else ()
            cmd += ("-o", go_file_out_path, go_file_in_path)
            subprocess.check_call(cmd)

def cmd_go_run(file: str):
    """
    Executes "go run" with development environment variables.

    Arguments:
      - file -- Path of the file to run.
    """
    check_build_configured(build_dir)
    info(f"Go: Running \"{file}\"...")
    cmd = ("go", "run", file)
    env = dict(os.environ, **get_dev_env_vars(load_cmake_cache_from_dir(build_dir)))
    subprocess.check_call(cmd, env=env)

def cmd_go_test(verbose: bool = False):
    """
    Executes "go test" with development environment variables.

    Arguments:
      - verbose -- Enables verbose mode which runs all tests.
    """
    check_build_configured(build_dir)
    info("Go: Testing...")
    cmd = ["go", "test"]
    if verbose:
        cmd.append("-v")
    env = dict(os.environ, **get_dev_env_vars(load_cmake_cache_from_dir(build_dir)))
    subprocess.check_call(cmd, env=env)

def cmd_test(timeout: float = 30):
    """Runs tests using CTest."""
    check_build_configured(build_dir)
    cmake_cache = load_cmake_cache_from_dir(build_dir)
    check_testing_enabled(cmake_cache)
    build_type = cmake_cache.get("CMAKE_BUILD_TYPE")
    info("Testing...")
    cmd = ["ctest", "--test-dir", build_dir, "--output-on-failure", "--timeout", str(timeout)]
    if build_type:
        cmd += ("--config", build_type)
    subprocess.check_call(cmd)

def main():
    prepend_tools_to_path_env(find_cmake, find_ninja)
    program_name = os.path.basename(__file__)
    spec = create_program_spec(program_name, "This is the build script for the webview library.", (
        cmd_build,
        cmd_clean,
        cmd_configure,
        cmd_devenv,
        cmd_go_build,
        cmd_go_run,
        cmd_go_test,
        cmd_test
    ))
    parse_args(sys.argv[1:], spec)

if __name__ == "__main__":
    main()
