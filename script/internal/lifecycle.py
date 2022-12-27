from internal.task import TaskRunner

from abc import abstractmethod
import sys


class LifecycleStrategy:
    @abstractmethod
    def configure_targets(self):
        pass

    @abstractmethod
    def configure_tasks(self, task_runner: TaskRunner):
        pass

    @abstractmethod
    def on_configured(self):
        pass

class Lifecycle:
    _strategy: LifecycleStrategy

    def __init__(self, strategy: LifecycleStrategy):
        self._strategy = strategy

    def run(self):
        task_runner = TaskRunner()
        self._strategy.configure_targets()
        self._strategy.configure_tasks(task_runner)
        self._strategy.on_configured()

        if task_runner.get_task_count() == 0:
            return

        def print_status(task_number: int, task_count: int, message: str):
            sys.stdout.write(
                "\r[{}/{}] {}\n".format(task_number, task_count, message))
        task_runner.execute(on_status=print_status)
