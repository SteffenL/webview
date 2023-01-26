from internal.build import BuildType, RuntimeLinkType
from internal.common import Arch
from internal.cli import BoolArgAction, HelpFormatter, IntArgAction, StrArgAction
from internal.options import LintMode
from internal.toolchain.common import ToolchainEnvironmentId, ToolchainId

import argparse
import platform


def create_arg_parser():
    system = platform.system()

    epilog = r"""Cross-compilation
=================

You must install dependencies for your target architecture onto your system.
Multilib support for x86-64 should be enough for x86/x64 cross-compilation.

You should set the target CPU architecture as a CLI option, and if needed
you can set a toolchain prefix which will be appended to executable names.

MinGW-w64 on Windows
====================

MinGW-w64 distributions may differ in support for compilers, host/target CPU
architectures, cross-compilation and so on. While this script tries hard to
find a matching toolchain, it may need some assistance.

MinGW-w64 is expected to be found in one of the following locations:
 - !SystemDrive!\
 - !SystemDrive!\msys64\
 - Chocolatey

One of the following subdirectories should exist depending on the target
architecture:
 - clangarm64
 - clang64
 - clang32
 - mingw
 - mingw32
 - mingw64
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
    group.add_argument("--build-dir",
                       help="Alternative build directory.",
                       action=StrArgAction)
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
                       choices=tuple(v.value for v in BuildType),
                       default=BuildType.RELEASE.value,
                       action=StrArgAction)
    group.add_argument("--runtime-link",
                       help="Runtime library linking.",
                       choices=tuple(v.value for v in RuntimeLinkType),
                       default=RuntimeLinkType.SHARED.value,
                       action=StrArgAction)
    group.add_argument("--target-arch",
                       help="Build for the target CPU architecture.",
                       choices=(tuple(v.value for v in Arch)),
                       default=Arch.NATIVE.value,
                       action=StrArgAction)
    group.add_argument("--toolchain-prefix",
                       help="Toolchain executable name prefix.",
                       action=StrArgAction)
    group.add_argument("--reformat",
                       help="Reformat code (requires clang-format).",
                       action=BoolArgAction)
    group.add_argument("--check",
                       help="Enable all checks.",
                       action=BoolArgAction)
    group.add_argument("--check-style",
                       help="Check code style (requires clang-format).",
                       action=BoolArgAction)
    group.add_argument("--check-lint",
                       help="Run lint checks (requires clang-tidy).",
                       choices=tuple(v.value for v in LintMode),
                       nargs="?",
                       default=LintMode.FALSE.value,
                       const=LintMode.STRICT.value,
                       action=StrArgAction)
    group.add_argument("--ar",
                       help="Archiver binary, e.g. ar or lib. Can be set by the AR environment variable.",
                       action=StrArgAction)
    group.add_argument("--cc",
                       help="C compiler binary, e.g. cc, gcc, clang or cl. Can be set by the CC environment variable.",
                       action=StrArgAction)
    group.add_argument("--cxx",
                       help="C++ compiler binary, e.g. c++, g++, clang++ or cl. Can be set by the CXX environment variable.",
                       action=StrArgAction)
    group.add_argument("--ld",
                       help="Linker binary, e.g. ld or link. Can be set by the LD environment variable.",
                       action=StrArgAction)
    group.add_argument("--go-build-examples",
                       help="Build Go examples.",
                       action=BoolArgAction)
    group.add_argument("--go-test",
                       help="Run Go tests.",
                       action=BoolArgAction)
    group.add_argument("--fetch-deps",
                        help="Fetch library dependencies. Implied when building and linting.",
                        action=BoolArgAction)
    group.add_argument("--target-triplet",
                       help="Clang target triplet.",
                       action=StrArgAction)
    group.add_argument("--max-workers",
                       help="Max parallel workers.",
                       action=IntArgAction)

    if system == "Windows":
        group.add_argument("--load-toolchain",
                           help="Attempt to load a C/C++ toolchain environment.",
                           choices=tuple(v.value for v in ToolchainEnvironmentId))
        group.add_argument("--mswebview2-version",
                           help="MS WebView2 version to use.",
                           default="1.0.1150.38")

    return parser
