from internal.task import TaskRunner, TaskStatus

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

        task_count = task_runner.get_task_count()
        task_number = 0

        if task_count == 0:
            return

        def print_status(status: TaskStatus, message: str, is_concurrent: bool, output: bytes, exception: Exception):
            nonlocal task_count
            nonlocal task_number
            if is_concurrent:
                if status == TaskStatus.STARTED:
                    print("[started] {}".format(message))
                    return
                elif status not in (TaskStatus.DONE, TaskStatus.FAILED):
                    return
            elif status not in (TaskStatus.STARTED, TaskStatus.FAILED):
                return
            task_number += 1
            color_error = "\033[91m"
            color_normal = "\033[00m"
            if output is not None:
                sys.stdout.buffer.write(output)
            if status == TaskStatus.FAILED:
                print(color_error + "[{}/{}] {} - FAILED".format(task_number, task_count, message) + color_normal)
                #raise exception
            elif status == TaskStatus.CANCELED:
                print(color_error + "[{}/{}] {} - CANCELED".format(task_number, task_count, message) + color_normal)
            else:
                print("[{}/{}] {}".format(task_number, task_count, message))

        task_runner.execute(on_status=print_status)
