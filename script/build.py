#!/usr/bin/env python3

import sys

min_required_python = (3, 7)
if sys.version_info < min_required_python:
    sys.exit("Python %d.%d or later is required." % min_required_python)
else:
    from cli import create_arg_parser
    from internal.common import Arch
    from internal.cli import parse_options
    from internal.lifecycle import Lifecycle
    from internal.options import format_option_value, Options
    from internal.task import TaskRunner
    from internal.toolchain.common import ToolchainEnvironmentId
    from internal.toolchain.toolchain import activate_toolchain, detect_toolchain, Toolchain
    from internal.utility import get_host_arch
    from internal.workspace import Workspace
    import targets.deps
    import targets.main
    import tasks.checks.lint
    import tasks.checks.style
    import tasks.clean
    import tasks.compile
    import tasks.generate.reformat
    import tasks.test
    import tasks.go
    from internal.target import Target
    import os
    from typing import Sequence


def pre_process_options(options: Options):
    # Enable all checks.
    if options.check.get_value():
        if not options.check_lint.is_explicit():
            options.check_lint.set_value(True)
        if not options.check_style.is_explicit():
            options.check_style.set_value(True)

    # Cannot specify only one of C/C++ compiler.
    if (options.cc.get_value() is None) != (options.cxx.get_value() is None):
        raise Exception(
            "Either both of cc/cxx options or neither must be specified")

    # Set the the host architecture if needed.
    if options.target_arch.get_value() == Arch.NATIVE:
        options.target_arch.set_value(get_host_arch())

    # Building Go examples requires building.
    if options.go_build_examples.get_value() and not options.go_build.is_explicit():
        options.go_build.set_value(True)

    # Running Go tests requires building.
    if options.go_test.get_value() and not options.go_build.is_explicit():
        options.go_build.set_value(True)

    # Building with Go requires dependencies.
    if options.go_build.get_value() and not options.fetch_deps.is_explicit():
        options.fetch_deps.set_value(True)

    # Enable building everything.
    if options.build.get_value():
        if not options.build_library.is_explicit():
            options.build_library.set_value(True)
        if not options.build_examples.is_explicit():
            options.build_examples.set_value(True)
        if not options.build_tests.is_explicit():
            options.build_tests.set_value(True)

    # Running tests requires building tests.
    if options.test.get_value() and not options.build_tests.is_explicit():
        options.build_tests.set_value(True)

    # Building examples requires building library.
    if options.build_examples.get_value() and not options.build_library.is_explicit():
        options.build_library.set_value(True)

    # Building tests requires building library.
    if options.build_tests.get_value() and not options.build_library.is_explicit():
        options.build_library.set_value(True)

    # Building library requires dependencies.
    if options.build_library.get_value() and not options.fetch_deps.is_explicit():
        options.fetch_deps.set_value(True)

    # Set the archiver binary based on the environment.
    if not options.ar.is_explicit():
        options.ar.set_value(os.environ.get("AR", None))

    # Set the C compiler binary based on the environment.
    if not options.cc.is_explicit():
        options.cc.set_value(os.environ.get("CC", None))

    # Set the C++ compiler binary based on the environment.
    if not options.cxx.is_explicit():
        options.cxx.set_value(os.environ.get("CXX", None))

    # Set the linker binary based on the environment.
    if not options.ld.is_explicit():
        options.ld.set_value(os.environ.get("LD", None))

    return options


def print_toolchain(toolchain: Toolchain):
    toolchain_binaries = toolchain.get_binaries()
    print("Toolchain:")
    print("  ID: {}".format(toolchain.get_id().value))
    print("  Archiver: {}".format(toolchain_binaries.ar))
    print("  C compiler: {}".format(toolchain_binaries.cc))
    print("  C++ compiler: {}".format(toolchain_binaries.cxx))
    print("  Linker: {}".format(toolchain_binaries.ld))


def print_targets(targets: Sequence[Target]):
    if len(targets) > 0:
        print("Targets:")
        for target in targets:
            print("  {}".format(target.get_name()))


def main(args):
    script_dir = os.path.dirname(__file__)
    source_dir = os.path.dirname(script_dir)
    options = pre_process_options(parse_options(args, create_arg_parser))
    architecture = options.target_arch.get_value()
    build_dir = options.build_dir.get_value(default="build")

    toolchain_env = options.load_toolchain.get_value()
    if toolchain_env is not None:
        activate_toolchain(toolchain_env, architecture)

    toolchain = detect_toolchain(architecture,
                                 toolchain_prefix=options.toolchain_prefix.get_value(),
                                 ar_override=options.ar.get_value(),
                                 cc_override=options.cc.get_value(),
                                 cxx_override=options.cxx.get_value(),
                                 ld_override=options.ld.get_value())

    workspace = Workspace(options,
                          toolchain,
                          source_dir=source_dir,
                          build_dir=build_dir,
                          build_type=options.build_type.get_value())

    task_runner = TaskRunner()

    targets.deps.register(task_runner, workspace)
    targets.main.register(task_runner, workspace)

    tasks.go.register(task_runner, workspace)
    tasks.generate.reformat.register(task_runner, workspace)
    tasks.checks.style.register(task_runner, workspace)
    tasks.checks.lint.register(task_runner, workspace)
    tasks.clean.register(task_runner, workspace)
    tasks.compile.register(task_runner, workspace)
    tasks.test.register(task_runner, workspace)

    print_toolchain(workspace.get_toolchain())
    print_targets(workspace.get_sorted_targets())

    print("Running tasks...")
    Lifecycle(task_runner).run()


if __name__ == "__main__":
    main(sys.argv[1:])
