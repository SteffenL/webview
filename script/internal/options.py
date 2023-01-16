from internal.build import BuildType
from internal.common import Arch
from internal.toolchain.common import ToolchainEnvironmentId, ToolchainId

from dataclasses import dataclass
from enum import Enum
from typing import Any, Generic, Mapping, TypeVar, Type


class LintMode(Enum):
    STRICT = "strict"
    LAX = "lax"
    FALSE = "false"


T = TypeVar("T")


@dataclass
class Option(Generic[T]):
    _value: T
    _explicit: bool

    def __init__(self, value: T, explicit: bool):
        self._value = value
        self._explicit = explicit

    def is_explicit(self):
        return self._explicit

    def get_value(self, default: T = None):
        return default if self._value is None else self._value

    def set_value(self, value: T):
        self._value = value


class Options():
    check: Option[bool]
    check_lint: Option[LintMode]
    check_style: Option[bool]
    clean: Option[bool]
    build: Option[bool]
    build_dir: Option[str]
    build_library: Option[bool]
    build_examples: Option[bool]
    build_tests: Option[bool]
    build_type: Option[BuildType]
    test: Option[bool]
    target_arch: Option[Arch]
    toolchain_prefix: Option[str]
    show_options: Option[bool]
    reformat: Option[bool]
    ar: Option[str]
    cc: Option[str]
    cxx: Option[str]
    ld: Option[str]
    go_build: Option[bool]
    go_build_examples: Option[bool]
    go_test: Option[bool]

    # Windows
    fetch_deps:  Option[bool]
    toolchain: Option[ToolchainId]
    load_toolchain: Option[ToolchainEnvironmentId]
    mswebview2_version: Option[str]


def format_option_value(value):
    if value is None:
        return ""
    if type(value) == bool:
        return str(value).lower()
    return str(value)
