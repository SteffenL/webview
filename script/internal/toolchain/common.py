from internal.build import Language
from internal.common import Arch
from internal.target import Target, TargetType
from internal.task import Task, TaskCollection

from abc import abstractmethod
from enum import Enum
from dataclasses import dataclass
import os
import subprocess
from typing import Iterable, MutableSequence, Sequence, Tuple


class ToolchainId(Enum):
    CLANG = "clang"
    GCC = "gcc"
    MSVC = "msvc"


class EnvironmentId(Enum):
    MSVC = "msvc"
    MINGW = "mingw"


@dataclass
class CompileParams:
    cflags: Sequence[str]
    input_path: str
    output_path: str

    def __init__(self):
        self.cflags = []


@dataclass
class LinkParams:
    ldflags: Sequence[str]
    input_paths: Sequence[str]
    output_path: str

    def __init__(self):
        self.ldflags = []
        self.input_paths = []


@dataclass
class ArchiveParams:
    input_paths: Sequence[str]
    output_path: str

    def __init__(self):
        self.input_paths = []


@dataclass
class ToolchainBinaries:
    ar: str
    cc: str
    cxx: str

    def __init__(self, ar: str = None, cc: str = None, cxx: str = None) -> None:
        self.ar = ar
        self.cc = cc
        self.cxx = cxx

class Toolchain:
    _id: ToolchainId
    _architecture: Arch
    _binaries: ToolchainBinaries

    def __init__(self, id: ToolchainId, architecture: Arch, binaries: ToolchainBinaries):
        self._id = id
        self._architecture = architecture
        self._binaries = binaries

    @staticmethod
    def _process_task(arg: Tuple[str, Sequence[str]]):
        output_path = arg[0]
        command = arg[1]
        if output_path is not None:
            os.makedirs(os.path.dirname(output_path), exist_ok=True)
        return subprocess.check_call(command)

    def create_compile_tasks(self, *targets: Target) -> Iterable[TaskCollection]:
        compile_tasks = TaskCollection(concurrent=True)
        archive_tasks = TaskCollection()
        link_tasks = TaskCollection()

        for target in targets:
            if not target.is_condition_met():
                continue
            compile_tasks.add_task(*self._create_compile_tasks(target))
            archive_tasks.add_task(*self._create_archive_tasks(target))
            link_tasks.add_task(*self._create_link_tasks(target))

        return (
            compile_tasks,
            archive_tasks,
            link_tasks
        )

    def _create_compile_tasks(self, target: Target) -> Sequence[Task]:
        if not target.get_type() in (TargetType.EXE, TargetType.OBJECT, TargetType.SHARED_LIBRARY, TargetType.STATIC_LIBRARY):
            return tuple()
        tasks: MutableSequence[Task] = []
        language = target.get_language()
        compile_exe = self.get_compile_exe(language)
        params_per_file = self.get_compile_params(target)
        for params in params_per_file:
            compile_command = (compile_exe, *self._format_compile_params(
                params, add_input=True, add_output=True))
            tasks.append(Task(self._process_task, arg=(params.output_path, compile_command),
                              description="Compile {}".format(params.input_path)))
        return tasks

    def _create_archive_tasks(self, target: Target) -> Sequence[Task]:
        if target.get_type() != TargetType.STATIC_LIBRARY:
            return tuple()
        language = target.get_language()
        params = self.get_archive_params(target)
        archive_exe = self.get_archive_exe(language)
        archive_command = (archive_exe, *self._format_archive_params(
            target.get_type(), params))
        return (Task(self._process_task, arg=(params.output_path, archive_command),
                     description="Archive target {}".format(target.get_name())),)

    def _create_link_tasks(self, target: Target) -> Sequence[Task]:
        if not target.get_type() in (TargetType.EXE, TargetType.OBJECT, TargetType.SHARED_LIBRARY):
            return tuple()
        language = target.get_language()
        params = self.get_link_params(target)
        link_exe = self.get_link_exe(language)
        link_command = (link_exe, *self._format_link_params(
            target.get_type(), params))
        return (Task(self._process_task, arg=(params.output_path, link_command),
                     description="Link target {}".format(target.get_name())),)

    @abstractmethod
    def get_compile_exe(self, language: Language):
        pass

    @abstractmethod
    def get_archive_exe(self, language: Language):
        pass

    @abstractmethod
    def get_link_exe(self, language: Language):
        pass

    @abstractmethod
    def get_archive_params(self, target: Target) -> ArchiveParams:
        pass

    @abstractmethod
    def get_compile_params(self, target: Target) -> Sequence[CompileParams]:
        pass

    @abstractmethod
    def get_link_params(self, target: Target) -> LinkParams:
        pass

    @abstractmethod
    def _format_compile_params(self, target: Target, params: CompileParams, add_input: bool = False, add_output: bool = False) -> Sequence[str]:
        pass

    @abstractmethod
    def _format_archive_params(self, params: LinkParams) -> Sequence[str]:
        pass

    @abstractmethod
    def _format_link_params(self, params: LinkParams) -> Sequence[str]:
        pass

    @abstractmethod
    def get_file_name_prefix(self, target_type: TargetType) -> str:
        pass

    def get_file_name_extension(self, target_type: TargetType, system: str) -> str:
        if target_type == TargetType.EXE:
            return {"Windows": ".exe"}.get(system, "")
        if target_type == TargetType.SHARED_LIBRARY:
            return {"Linux": ".so",
                    "Darwin": ".dylib",
                    "Windows": ".dll"}.get(system, "")
        return ""

    def get_id(self):
        return self._id

    def get_architecture(self):
        return self._architecture

    def get_binaries(self):
        return self._binaries
