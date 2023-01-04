from internal.build import find_c_like_source_files
from internal.workspace import Workspace
from internal.task import Task, TaskPhase, TaskRunner
from internal.utility import execute_program, find_executable


def reformat(task: Task, arg):
    """Reformats C/C++ source code files in the source directory."""

    exe: str
    source: str

    exe, source = arg

    command = (exe, "-i", source)
    execute_program(command, pipe_output=True)


def register(task_runner: TaskRunner, workspace: Workspace):
    """Registers tasks for reformatting code."""

    if not workspace.get_options().reformat.get_value():
        return

    exe = find_executable("clang-format", required=True)
    tasks = task_runner.create_task_collection(
        TaskPhase.GENERATE, concurrent=True)
    sources = find_c_like_source_files(
        workspace.get_source_dir(), include_headers=True)
    for _, source in sources:
        tasks.add_task(Task(reformat, arg=(exe, source),
                            description="Reformat {}".format(source)))
