from internal.workspace import Workspace
from internal.task import Task, TaskPhase, TaskRunner
from internal.test import Test

from internal.build import FileType, find_sources, Language, PropertyScope
from internal.common import Arch
from internal.go import go_version_supports_quoted_params, parse_go_version_string, to_go_architecture
from internal.target import get_file_extension_for_target_type, TargetType
from internal.toolchain.common import Toolchain, ToolchainId
from internal.toolchain.toolchain import activate_toolchain, detect_toolchain
from internal.utility import execute_program, get_host_arch

import os
import platform
from typing import Iterable, Mapping


def get_env(workspace: Workspace, toolchain: Toolchain):
    # Argument quoting only works for Go 1.18 and later.
    # We therefore conditionally quote arguments in e.g. CGO_CXXFLAGS
    # and CGO_LDFLAGS based on the version of Go.
    go_version_string = execute_program(
        ("go", "version"), pipe_output=True).get_output_string()
    go_version = parse_go_version_string(go_version_string)
    use_quotes = go_version_supports_quoted_params(go_version)
    quote = '"' if use_quotes else ""

    arch = toolchain.get_architecture()
    includes = []
    exe_search_paths = []

    if platform.system() == "Windows":
        mswebview2 = workspace.get_target("mswebview2")
        includes += mswebview2.get_include_dirs(PropertyScope.EXTERNAL)
        exe_search_paths.append(workspace.get_lib_dir())

    cxxflags = []
    cxxflags += tuple(f"{quote}-I{s}{quote}" for s in includes)

    env = {}
    env.update(os.environ)
    env.update({
        "CGO_ENABLED": "1",
        "CC": toolchain.get_compile_exe(Language.C),
        "CXX": toolchain.get_compile_exe(Language.CXX),
    })
    if arch != Arch.NATIVE:
        env["GOARCH"] = to_go_architecture(arch)
    if len(cxxflags) > 0:
        env["CGO_CXXFLAGS"] = " ".join(cxxflags)
    if len(exe_search_paths) > 0:
        env["PATH"] = os.pathsep.join((*env["PATH"].split(os.pathsep), *exe_search_paths))
    return env


def go_cmd(task: Task, workspace: Workspace, command: Iterable[str], env: Mapping[str, str], working_dir: str):
    result = execute_program(
        command, env=env, working_dir=working_dir, pipe_output=True, ignore_error=True)
    task.set_result(result.get_output_string())
    if result.exit_code != 0:
        raise Exception("Command failed: {}".format(command))


def build_condition(task: Task, workspace: Workspace, *args):
    return workspace.get_options().go_build_examples.get_value()


def test_condition(workspace: Workspace):
    return workspace.get_options().go_test.get_value()


def register(task_runner: TaskRunner, workspace: Workspace):
    build_tasks = task_runner.create_task_collection(
        TaskPhase.COMPILE, concurrent=True)

    toolchain = workspace.get_toolchain()
    source_dir = workspace.get_source_dir()
    env = get_env(workspace, toolchain)

    # Examples
    examples_dir = os.path.join(source_dir, "examples")
    sources = find_sources(examples_dir, (FileType.GO,),
                           relative_to=source_dir)
    for _, source in sources:
        full_source_path = os.path.join(source_dir, source)
        output_file_ext = get_file_extension_for_target_type(
            TargetType.EXE, platform.system())
        output_file_name = "".join(("example_", os.path.basename(
            source).replace(".", "_"), output_file_ext))
        output_file = os.path.join(
            workspace.get_bin_dir(), output_file_name)
        build_command = ("go", "build", "-o", output_file, source)
        build_tasks.add_task(Task(go_cmd,
                                  args=(workspace, build_command,
                                        env, source_dir),
                                  description=f"Build Go example {full_source_path}",
                                  condition=build_condition))
    # Test
    workspace.add_test("go",
                       ("test", "-v"),
                       environment=env,
                       working_dir=source_dir,
                       description="Go bindings",
                       condition=lambda *_: test_condition(workspace))
