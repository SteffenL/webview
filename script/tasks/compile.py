from internal.workspace import Workspace
from internal.task import Task, TaskPhase, TaskRunner


def register(task_runner: TaskRunner, workspace: Workspace):
    """Registers tasks for target compilation."""

    targets = workspace.get_sorted_targets()
    toolchain = workspace.get_toolchain()
    task_runner.add_task_collection(
        TaskPhase.COMPILE, *toolchain.create_compile_tasks(*targets))
