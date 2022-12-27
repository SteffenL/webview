from internal.options import LintMode
from internal.target import TargetType
from internal.task import Task, TaskRunner
from internal.toolchain.clang_like import ClangLikeToolchain
from internal.toolchain.common import ToolchainBinaries, ToolchainId
from internal.workspace import Workspace

import subprocess


def register(task_runner: TaskRunner, workspace: Workspace):
    """Registers tasks for lint checks."""

    if workspace.get_options().check_lint.get_value() == LintMode.FALSE:
        return

    arch = workspace.get_toolchain().get_architecture()
    toolchain = ClangLikeToolchain(ToolchainId.CLANG, arch, ToolchainBinaries(
        ar="ar", cc="clang", cxx="clang++"))
    tasks = task_runner.create_task_collection(concurrent=True)

    for target in workspace.get_targets(all=True):
        if target.get_type() == TargetType.INTERFACE:
            continue

        for compile_params in toolchain.get_compile_params(target):
            tidy_params = []

            if workspace.get_options().check_lint.get_value() == LintMode.STRICT:
                tidy_params += ("--warnings-as-errors=*",)

            args = ["clang-tidy", "--quiet"]
            args += tidy_params
            args.append(compile_params.input_path)
            args.append("--")
            args += compile_params.cflags

            tasks.add_task(Task(
                lambda args: subprocess.check_call(args),
                arg=args,
                description="Lint target {} ({})".format(target.get_name(), compile_params.input_path)))
