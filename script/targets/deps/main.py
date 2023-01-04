from internal.task import TaskRunner
from internal.workspace import Workspace
import targets.deps.mswebview2

import platform


def register(task_runner: TaskRunner, workspace: Workspace):
    """Registers all dependencies."""

    system = platform.system()
    if system == "Windows":
        targets.deps.mswebview2.register(task_runner, workspace)
