from internal.utility import execute_program

import os
from typing import Callable, Iterable, Mapping


class Test():
    _executable: str
    _arguments: Iterable[str]
    _environment: Mapping[str, str]
    _condition: Callable[[], bool]
    _working_dir: str
    _description: str

    def __init__(self, executable: str,
                 arguments: Iterable[str] = None,
                 environment: Mapping[str, str] = None,
                 condition: Callable[[], bool] = None,
                 working_dir: str = None,
                 description: str = None):
        self._executable = executable
        self._arguments = [] if arguments is None else arguments
        self._environment = {}
        if environment is not None:
            self._environment.update(environment)
        self._condition = (lambda: True) if condition is None else condition
        self._working_dir = working_dir
        self._description = description

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

    def get_working_dir(self):
        return self._working_dir

    def get_description(self):
        if self._description is None:
            return self.get_executable()
        return self._description

    def run(self):
        command = (self.get_executable(), *self.get_arguments())
        env = dict()
        env.update(os.environ)
        env.update(self.get_environment())
        return execute_program(command,
                               env=env,
                               working_dir=self.get_working_dir(),
                               pipe_output=True,
                               ignore_error=True)
