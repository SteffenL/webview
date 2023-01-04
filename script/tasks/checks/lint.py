from internal.options import LintMode
from internal.target import TargetType
from internal.task import Task, TaskPhase, TaskRunner
from internal.toolchain.clang_like import ClangLikeToolchain
from internal.toolchain.common import ToolchainBinaries, ToolchainId
from internal.utility import execute_program, find_executable
from internal.workspace import Workspace

from typing import Iterable


def register(task_runner: TaskRunner, workspace: Workspace):
    """Registers tasks for lint checks."""

    if workspace.get_options().check_lint.get_value() == LintMode.FALSE:
        return

    arch = workspace.get_toolchain().get_architecture()
    toolchain = ClangLikeToolchain(ToolchainId.CLANG, arch, ToolchainBinaries(
        ar="ar", cc="clang", cxx="clang++"))
    tasks = task_runner.create_task_collection(
        TaskPhase.CHECK, concurrent=True)

    def check(task: Task, command: Iterable[str]):
        result = execute_program(command, pipe_output=True, ignore_error=True)
        task.set_result(result.get_output_string())
        if result.exit_code != 0:
            raise Exception("Command failed: {}".format(command))

    exe = find_executable("clang-tidy", required=True)

    for target in workspace.get_targets(all=True):
        if target.get_type() == TargetType.INTERFACE:
            continue

        for compile_params in toolchain.get_compile_params(target):
            tidy_params = []

            if workspace.get_options().check_lint.get_value() == LintMode.STRICT:
                tidy_params += ("--warnings-as-errors=*",)

            command = [exe, "--quiet"]
            command += tidy_params
            command.append(compile_params.input_path)
            command.append("--")
            command += compile_params.cflags

            tasks.add_task(Task(
                check,
                arg=command,
                description="Lint target {} ({})".format(target.get_name(), compile_params.input_path)))
