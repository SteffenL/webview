from internal.workspace import Workspace
from internal.task import Task, TaskRunner
from internal.test import Test


def run_test(task: Task, test: Test):
    result = test.run()
    task.set_result(result.get_output_string())
    if result.exit_code != 0:
        raise Exception("Test failed: {}".format(test.get_executable()))


def register(task_runner: TaskRunner, workspace: Workspace):
    """Registers tasks for running tests."""

    test_tasks = task_runner.create_task_collection()
    for test in workspace.get_tests():
        test_tasks.add_task(Task(run_test, arg=test,
                            description="Test {}".format(test.get_executable())))
