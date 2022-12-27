from internal.build import find_c_like_source_files
from internal.workspace import Workspace
from internal.task import Task, TaskRunner

import subprocess


def generate_task(workspace: Workspace):
    """Reformats C/C++ source code files in the source directory."""

    sources = find_c_like_source_files(
        workspace.get_source_dir(), include_headers=True)
    for _, source in sources:
        print("Reformatting {}...".format(source))
        subprocess.check_call(("clang-format", "-i", source))


def generate_condition(workspace: Workspace):
    return workspace.get_options().reformat.get_value()


def register(task_runner: TaskRunner, workspace: Workspace):
    """Registers tasks for reformatting code."""

    tasks = task_runner.create_task_collection()
    tasks.add_task(Task(generate_task, arg=workspace,
                   description="Reformat code", condition=generate_condition))
