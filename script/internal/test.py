from internal.utility import execute_program

import os
from typing import Callable, Iterable, Mapping


class Test():
    _executable: str
    _arguments: Iterable[str]
    _environment: Mapping[str, str]
    _condition: Callable[[], bool]

    def __init__(self, executable: str,
                 arguments: Iterable[str] = None,
                 environment: Mapping[str, str] = None):
        self._executable = executable
        self._arguments = [] if arguments is None else arguments
        self._environment = {}
        if environment is not None:
            self._environment.update(environment)
        self._condition = lambda: True

    def get_executable(self):
        return self._executable

    def get_arguments(self):
        return tuple(self._arguments)

    def get_environment(self):
        result = {}
        result.update(self._environment)
        return result

    def set_condition(self, condition: Callable[[], bool]):
        self._condition = condition

    def is_condition_met(self):
        return self._condition()

    def run(self):
        command = (self.get_executable(), *self.get_arguments())
        env = dict()
        env.update(os.environ)
        env.update(self.get_environment())
        return execute_program(command, env=env, pipe_output=True, ignore_error=True)
