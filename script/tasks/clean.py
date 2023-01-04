from internal.workspace import Workspace
from internal.task import Task, TaskPhase, TaskRunner

import shutil


def clean_build_dir(task: Task, workspace: Workspace):
    dir = workspace.get_build_arch_dir()
    if dir is None or len(dir) == 0:
        return
    try:
        shutil.rmtree(dir)
    except FileNotFoundError:
        pass


def register(task_runner: TaskRunner, workspace: Workspace):
    """Registers cleaning tasks."""

    if not workspace.get_options().clean.get_value():
        return

    tasks = task_runner.create_task_collection(TaskPhase.CLEAN)
    tasks.add_task(Task(clean_build_dir, arg=workspace,
                   description="Clean build directory"))
