from internal.workspace import Workspace
from internal.task import Task, TaskPhase, TaskRunner
from internal.test import Test

from internal.build import FileType, find_sources, Language
from internal.common import Arch
from internal.options import Options
from internal.target import get_file_extension_for_target_type, TargetType
from internal.toolchain.common import Toolchain, ToolchainId
from internal.toolchain.toolchain import activate_toolchain, detect_toolchain
from internal.utility import execute_program

import os
import platform
import subprocess
from typing import Iterable, Mapping


def get_env(arch: Arch, toolchain: Toolchain):
    go_arch = "amd64" if arch == Arch.X64 else "x86" if arch == Arch.X86 else None
    env = {}
    env.update(os.environ)
    env.update({
        "GOARCH": go_arch,
        "CGO_ENABLED": "1",
        "CC": toolchain.get_compile_exe(Language.C),
        "CXX": toolchain.get_compile_exe(Language.CXX)
    })
    return env


def go_cmd(task: Task, workspace: Workspace, command: Iterable[str], env: Mapping[str, str], working_dir: str):
    result = execute_program(
        command, env=env, working_dir=working_dir, pipe_output=True, ignore_error=True)
    task.set_result(result.get_output_string())
    if result.exit_code != 0:
        raise Exception("Command failed: {}".format(command))


def build_condition(task: Task, workspace: Workspace, *args):
    return workspace.get_options().go_build_examples.get_value()


def test_condition(task: Task, workspace: Workspace, *args):
    return workspace.get_options().go_test.get_value()


def register(task_runner: TaskRunner, workspace: Workspace):
    build_tasks = task_runner.create_task_collection(
        TaskPhase.GO_BUILD, concurrent=True)
    test_tasks = task_runner.create_task_collection(TaskPhase.GO_TEST)

    toolchain = workspace.get_toolchain()
    source_dir = workspace.get_source_dir()
    env = get_env(toolchain.get_architecture(), toolchain)

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
    test_command = ("go", "test", "-v")
    test_tasks.add_task(Task(go_cmd,
                             args=(workspace, test_command, env, source_dir),
                             description=f"Run Go test",
                             condition=test_condition))
