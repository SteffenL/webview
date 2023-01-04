from internal.cli import terminal_supports_escape, PrintColor, PrintSequenceBuilder
from internal.task import TaskRunner, TaskStatus

import sys


class Lifecycle:
    _status_width = max(len(s.value) for s in TaskStatus)
    _status_colors = {
        TaskStatus.CANCELED: PrintColor.FG_RED,
        TaskStatus.FINISHED: PrintColor.FG_GREEN,
        TaskStatus.FAILED: PrintColor.FG_RED,
        TaskStatus.STARTED: PrintColor.FG_BLUE
    }

    _task_runner: TaskRunner

    def __init__(self, task_runner: TaskRunner):
        self._task_runner = task_runner

    def run(self):
        task_count = self._task_runner.get_task_count()
        task_number = 0

        if task_count == 0:
            return

        interactive = terminal_supports_escape()
        task_count_digits = len(str(task_count))
        max_progress_length = len("[{}/{}]".format("9".rjust(task_count_digits),
                                                   "9".rjust(task_count_digits)))

        def on_status(status: TaskStatus, message: str, is_concurrent: bool, output: str, exception: Exception):
            nonlocal task_count
            nonlocal task_number
            nonlocal task_count_digits
            nonlocal max_progress_length

            if status in (TaskStatus.CANCELED, TaskStatus.FINISHED, TaskStatus.FAILED):
                task_number += 1

            builder = PrintSequenceBuilder()
            builder.delete_line()
            if output is not None and len(output) > 0:
                builder.raw(output)
                if not output.endswith("\n"):
                    builder.raw("\n")
            show_progress = status != TaskStatus.STARTED
            if show_progress:
                builder.raw("[{}/{}]".format(str(task_number).rjust(task_count_digits),
                                             str(task_count).rjust(task_count_digits)))
            else:
                builder.raw(" " * max_progress_length)
            builder.raw(" ")
            if status is not None:
                builder.colored(
                    self._status_colors[status], status.value.upper().ljust(self._status_width))
                builder.raw(" ")
            builder.raw(message)
            builder.raw("\n")
            builder.progress_bar("Progress", task_number, task_count)
            builder.to_sequence(with_commands=interactive).print()

            if exception is not None:
                raise exception

        print("Running tasks...")
        try:
            self._task_runner.execute(on_status=on_status)
        finally:
            sys.stdout.flush()
