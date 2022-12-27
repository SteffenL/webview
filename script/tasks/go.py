from internal.build import FileType, find_sources, Phase
from internal.common import Arch
from internal.context import Context
from internal.options import Options
from internal.target import get_file_extension_for_target_type, TargetType
from internal.toolchain.common import Toolchain, ToolchainId
from internal.toolchain.toolchain import activate_toolchain, detect_toolchain

import os
import platform
import subprocess
from typing import Callable


def get_env(arch: Arch, toolchain: Toolchain):
    go_arch = "amd64" if arch == Arch.X64 else "x86" if arch == Arch.X86 else None
    env = {}
    env.update(os.environ)
    env.update({
        "GOARCH": go_arch,
        "CGO_ENABLED": "1",
        "CC": toolchain.cc.get_compiler_exe(),
        "CXX": toolchain.cxx.get_compiler_exe()
    })
    return env


def continue_with_toolchain(options: Options, callback: Callable[[Toolchain], None]):
    toolchain = detect_toolchain(cc_override=options.cc.get_value(),
                                 cxx_override=options.cxx.get_value())
    callback(toolchain)


def compile_task(context: Context, toolchain: Toolchain):
    source_dir = context.get_source_dir()
    examples_dir_name = "examples"
    examples_dir = os.path.join(source_dir, examples_dir_name)
    env = get_env(context.get_target_arch(), toolchain)
    sources = find_sources(examples_dir, (FileType.GO,),
                           relative_to=source_dir)
    for _, source in sources:
        output_file_ext = get_file_extension_for_target_type(
            TargetType.EXE, platform.system())
        output_file_name = os.path.splitext(os.path.basename(source))[
            0] + output_file_ext
        output_file = os.path.join(
            context.get_build_arch_dir(), examples_dir_name, "go", output_file_name)
        args = ("go", "build", "-o", output_file, source)
        subprocess.check_call(args, env=env)


def test_task(context: Context, toolchain: Toolchain):
    env = get_env(context.get_target_arch(), toolchain)
    args = ("go", "test", "-v")
    subprocess.check_call(args, env=env)


def wrap_task(context: Context, callback: Callable[[Context], None]):
    options = context.get_options()
    arch = context.get_target_arch()
    toolchain_id = options.toolchain.get_value()
    if platform.system() == "Windows":
        toolchain_id = ToolchainId.MINGW
    if toolchain_id is not None:
        # If a toolchain has been specified (implicitly or explicitly) then attempt
        # to activate its environment before using the toolchain.
        activate_toolchain(toolchain_id, arch,
                           lambda: continue_with_toolchain(
                               options, lambda toolchain: callback(context, toolchain)))
        return
    continue_with_toolchain()


def compile_condition(context: Context):
    return context.get_options().go_build_examples.get_value()


def test_condition(context: Context):
    return context.get_options().go_test.get_value()


def register(context: Context):
    context.add_task(Phase.COMPILE, "build go examples",
                     lambda context: wrap_task(context, compile_task),
                     condition=compile_condition)
    context.add_task(Phase.TEST, "run go tests",
                     lambda context: wrap_task(context, test_task),
                     condition=test_condition)
