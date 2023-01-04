from internal.task import TaskRunner
from internal.workspace import Workspace
import targets.deps.mswebview2

import platform


def register(workspace: Workspace):
    """Registers all dependencies."""

    system = platform.system()
    if system == "Windows":
        targets.deps.mswebview2.register(workspace)
