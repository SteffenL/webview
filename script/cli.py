from internal.build import BuildType
from internal.common import Arch
from internal.cli import BoolArgAction, HelpFormatter, StrArgAction
from internal.options import LintMode
from internal.toolchain.common import ToolchainId

import argparse
import platform


def create_arg_parser():
    system = platform.system()

    epilog = r"""Compilation with MinGW-w64
==========================

Unless your MinGW-w64 toolchain has multilib support then you need to
install both the 64-bit- and 32-bit toolchains for cross-compilation.
MinGW-w64 is expected to be found in one of the following locations:
 - !SystemDrive!\mingw64 or !SystemDrive!\mingw32
 - !SystemDrive!\msys64\mingw64 or !SystemDrive!\msys64\mingw32
 - Chocolatey
""" if system == "Windows" else r"""

Cross-compilation
=================

Your toolchain must have multilib support. In addition you must install
dependencies for your target architecture onto your system.
"""
    parser = argparse.ArgumentParser(
        prog="build",
        description="This is the build script for the webview library.",
        epilog=epilog,
        formatter_class=HelpFormatter,
        add_help=False
    )

    group = parser.add_argument_group("Options")

    group.add_argument("--help",
                       help="Display this help text.",
                       action="help")
    group.add_argument("--clean",
                       help="Clean the build directory.",
                       action=BoolArgAction)
    group.add_argument("--build",
                       help="Build everything.",
                       action=BoolArgAction)
    group.add_argument("--build-library",
                       help="Build library.",
                       action=BoolArgAction)
    group.add_argument("--build-examples",
                       help="Build examples (implies --build-library).",
                       action=BoolArgAction)
    group.add_argument("--build-tests",
                       help="Build tests (implies --build-library).",
                       action=BoolArgAction)
    group.add_argument("--test",
                       help="Run tests (implies --build-tests).",
                       action=BoolArgAction)
    group.add_argument("--build-type",
                       help="Build type dictates code optimization.",
                       choices=(tuple(v.value for v in BuildType)),
                       default=BuildType.RELEASE.value,
                       action=StrArgAction)
    group.add_argument("--target-arch",
                       help="Build for the target CPU architecture.",
                       choices=(tuple(v.value for v in Arch)),
                       default=Arch.NATIVE.value,
                       action=StrArgAction)
    group.add_argument("--reformat",
                       help="Reformat code (requires clang-format).",
                       action=BoolArgAction)
    group.add_argument("--check-style",
                       help="Check code style (requires clang-format).",
                       action=BoolArgAction)
    group.add_argument("--check-lint",
                       help="Run lint checks (requires clang-tidy).",
                       choices=(tuple(v.value for v in LintMode)),
                       nargs="?",
                       default=LintMode.FALSE.value,
                       const=LintMode.STRICT.value,
                       action=StrArgAction)
    group.add_argument("--cc",
                       help="C compiler binary, e.g. cc, gcc or clang. Can be set by the CC environment variable.",
                       action=StrArgAction)
    group.add_argument("--cxx",
                       help="C++ compiler binary, e.g. c++, g++ or clang++. Can be set by the CXX environment variable.",
                       action=StrArgAction)
    group.add_argument("--go-build-examples",
                       help="Build Go examples.",
                       action=BoolArgAction)
    group.add_argument("--go-test",
                       help="Run Go tests.",
                       action=BoolArgAction)

    if system == "Windows":
        group.add_argument("--fetch-deps",
                           help="Fetch library dependencies. Implied when building and linting.",
                           action=BoolArgAction)
        group.add_argument("--toolchain",
                           help="C/C++ toolchain.",
                           choices=(tuple(v.value for v in ToolchainId)),
                           default=ToolchainId.MSVC.value)
        group.add_argument("--mswebview2-version",
                           help="MS WebView2 version to use.",
                           default="1.0.1150.38")

    return parser
