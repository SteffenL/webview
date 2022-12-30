from internal.workspace import Workspace
from internal.task import Task, TaskRunner


def register(task_runner: TaskRunner, workspace: Workspace):
    """Registers tasks for running tests."""

    test_tasks = task_runner.create_task_collection()
    for test in workspace.get_tests():
        test_tasks.add_task(Task(lambda *_: test.run(),
                            description="Test {}".format(test.get_executable())))
