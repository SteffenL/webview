from internal.workspace import Workspace
from internal.task import TaskRunner


def register(task_runner: TaskRunner, workspace: Workspace):
    """Registers tasks for target compilation."""

    targets = workspace.get_sorted_targets()
    toolchain = workspace.get_toolchain()
    task_runner.add_task_collection(*toolchain.create_compile_tasks(*targets))
