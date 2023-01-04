from internal.build import find_c_like_source_files
from internal.workspace import Workspace
from internal.task import Task, TaskPhase, TaskRunner

import os
import subprocess


def check_file(task: Task, file_path: str):
    clang_format_args = ["clang-format", "--Werror", file_path]
    with subprocess.Popen(clang_format_args, stdout=subprocess.PIPE, stderr=subprocess.PIPE) as clang_format:
        git_diff_args = ["git", "diff", "--no-index", "--", file_path, "-"]
        with subprocess.Popen(git_diff_args, stdin=clang_format.stdout, stdout=subprocess.PIPE, stderr=subprocess.PIPE) as git_diff:
            clang_format.stdout.close()
            output = git_diff.communicate()[0]
            if output:
                task.set_result(output.decode("utf8"))
                raise Exception("Code style violation:\n" + output)


def register(task_runner: TaskRunner, workspace: Workspace):
    """Registers tasks for code style checks."""

    if not workspace.get_options().check_style.get_value():
        return

    tasks = task_runner.create_task_collection(
        TaskPhase.VALIDATE, concurrent=True)
    sources = find_c_like_source_files(
        workspace.get_source_dir(), include_headers=True)
    for _, source in sources:
        full_path = os.path.join(workspace.get_source_dir(), source)
        tasks.add_task(Task(check_file, arg=full_path,
                            description="Check code style ({})".format(full_path)))
