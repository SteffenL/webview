from internal.task import TaskRunner, TaskStatus

from abc import abstractmethod
import math
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

        color_red = "\033[31m"
        color_gray = "\033[34m"
        color_green = "\033[32m"
        color_reset = "\033[00m"
        bgcolor_white = "\033[47m"

        task_count_digits = len(str(task_count))
        max_progress_length = len("[{}/{}]".format("9".rjust(task_count_digits),
                                                   "9".rjust(task_count_digits)))

        status_width = max(len(s.value) for s in TaskStatus)
        status_colors = {
            TaskStatus.CANCELED: color_red,
            TaskStatus.FINISHED: color_green,
            TaskStatus.FAILED: color_red,
            TaskStatus.STARTED: color_gray
        }

        def on_status(status: TaskStatus, message: str, is_concurrent: bool, output: bytes, exception: Exception):
            nonlocal task_count
            nonlocal task_number
            nonlocal task_count_digits
            nonlocal max_progress_length
            nonlocal status_colors

            def print_status(show_progress: bool = False, status: TaskStatus = None, color: str = None, suffix: str = ""):
                nonlocal task_number
                line = ""
                if output is not None and len(output) > 0:
                    line += output
                    if not line.endswith("\n"):
                        line += "\n"
                if color is not None:
                    line += color
                if show_progress:
                    line += "[{}/{}]".format(str(task_number).rjust(task_count_digits),
                                             str(task_count).rjust(task_count_digits))
                else:
                    line += " " * max_progress_length
                line += " "
                if status is not None:
                    line += "".join((status_colors[status], status.value.upper().ljust(
                        status_width), color_reset, " "))
                line += message
                line += suffix
                if color is not None:
                    line += color_reset
                sys.stdout.write("\033[M" + line + "\n\033[L\033[s")
                progress_max_width = 40
                progress_width = math.floor(
                    (task_number / float(task_count)) * progress_max_width)
                sys.stdout.write("\033[9999EProgress: {} [{}]".format(
                    (str(math.floor((task_number / float(task_count)) * 100)) + "%").ljust(4),
                    bgcolor_white + (" " * progress_width) + color_reset +
                    (" " * (progress_max_width - progress_width))
                ))
                sys.stdout.flush()
                sys.stdout.write("\033[M\033[u")

            if status in (TaskStatus.CANCELED, TaskStatus.FINISHED, TaskStatus.FAILED):
                task_number += 1

            print_status(show_progress=status !=
                         TaskStatus.STARTED, status=status)

            # if status == TaskStatus.FAILED and exception is not None:
            #    raise exception

        print("Running tasks...")
        task_runner.execute(on_status=on_status)
        # sys.stdout.write("\033[s\033[999E\033[M\033[u")
        sys.stdout.flush()
