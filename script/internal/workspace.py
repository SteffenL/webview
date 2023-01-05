from internal.build import BuildType, PropertyScopes
from internal.options import Options
from internal.target import Target, TargetType
from internal.test import Test
from internal.toolchain.common import Toolchain

import os
from typing import Callable, Iterable, Mapping, MutableMapping, MutableSequence, Sequence, Union


class Workspace:
    _options: Options
    _source_dir: str
    _build_root_dir: str
    _targets: MutableMapping[str, Target]
    _toolchain: Toolchain
    _build_type: BuildType
    _tests: MutableSequence[Test]

    def __init__(self,
                 options: Options,
                 toolchain: Toolchain,
                 source_dir: str = os.getcwd(),
                 build_dir: str = "build",
                 build_type: BuildType = BuildType.RELEASE):
        source_dir = os.path.normpath(source_dir)
        source_dir = source_dir if os.path.isabs(
            source_dir) else os.path.join(os.getcwd(), source_dir)
        build_dir = build_dir if os.path.isabs(
            build_dir) else os.path.join(source_dir, build_dir)

        self._options = options
        self._source_dir = source_dir
        self._build_root_dir = build_dir
        self._targets = {}
        self._toolchain = toolchain
        self._build_type = build_type
        self._tests = []

    def get_options(self):
        return self._options

    def get_source_dir(self):
        return self._source_dir

    def get_build_root_dir(self):
        return self._build_root_dir

    def get_build_arch_dir(self):
        arch = self._toolchain.get_architecture()
        return os.path.join(self.get_build_root_dir(), arch.value)

    def get_build_type(self):
        return self._build_type

    def get_bin_dir(self):
        return os.path.join(self.get_build_arch_dir(), "bin")

    def get_lib_dir(self):
        return os.path.join(self.get_build_arch_dir(), "lib")

    def get_obj_dir(self, target: Target):
        return os.path.join(self.get_build_arch_dir(), "obj", target.get_name())

    def get_target(self, name: str):
        return self._targets[name]

    def get_targets(self, all: bool = False) -> Iterable[Target]:
        return tuple(filter(lambda target: all or target.is_condition_met(), self._targets.values()))

    def add_target(self, type: TargetType, name: str):
        if name in self._targets:
            raise Exception(
                "A target with the same name already exists: " + name)
        target = Target(self, type, name)
        self._targets[name] = target
        return target

    def get_toolchain(self):
        return self._toolchain

    def get_sorted_targets(self, all: bool = False) -> Sequence[Target]:
        targets = self.get_targets(all=True)
        sorted_targets = list(targets)
        for target in targets:
            target_index = sorted_targets.index(target)
            library_targets = tuple(t for t in target.get_link_libraries(
                PropertyScopes.PUBLIC) if isinstance(t, Target))
            for lib in library_targets:
                lib_index = sorted_targets.index(lib)
                if target_index >= lib_index:
                    continue
                moved_target = sorted_targets[lib_index]
                sorted_targets.remove(moved_target)
                sorted_targets.insert(target_index, moved_target)
        return tuple(filter(lambda target: all or target.is_condition_met(), sorted_targets))

    def add_test(self,
                 executable: Union[Target, str],
                 arguments: Iterable[str] = None,
                 environment: Mapping[str, str] = None,
                 condition: Callable[[], bool] = None,
                 working_dir: str = None,
                 description: str = None) -> Test:
        executable = executable.get_output_file_path() if isinstance(
            executable, Target) else executable
        test = Test(executable,
                    arguments=arguments,
                    environment=environment,
                    condition=condition,
                    working_dir=working_dir,
                    description=description)
        self._tests.append(test)
        return test

    def get_tests(self, all: bool = False) -> Sequence[Test]:
        return tuple(filter(lambda test: all or test.is_condition_met(), self._tests))
