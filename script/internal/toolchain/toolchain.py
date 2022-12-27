from internal.common import Arch
from internal.target import TargetType
from internal.utility import find_executable
#from internal.toolchain.clang_like import ClangLikeCompiler, is_clang_like_compiler
from internal.toolchain.clang_like import ClangLikeToolchain
from internal.toolchain.common import Toolchain, ToolchainBinaries, ToolchainId
from internal.toolchain.mingw import activate_mingw_toolchain
from internal.toolchain.msvc import activate_msvc_toolchain, MsvcToolchain

from dataclasses import dataclass
import os
import platform
import subprocess
from typing import Callable, Mapping, Tuple, Type


def activate_toolchain(id: ToolchainId, arch: Arch):
    system = platform.system()
    if system == "Windows":
        if id == ToolchainId.MSVC:
            activate_msvc_toolchain(arch)
            return
        if id in (ToolchainId.GCC, ToolchainId.CLANG):
            activate_mingw_toolchain(arch)
            return


def detect_toolchain(architecture: Arch, cc_override: str = None, cxx_override: str = None) -> Toolchain:
    if (cc_override is None) != (cxx_override is None):
        raise Exception(
            "Either all overrides or none must be specified")

    cc: str = None
    cxx: str = None

    if cc_override is not None:
        cc = find_executable(cc_override)
        if cc is None:
            raise Exception("C compiler not found: {}".format(cc_override))

    if cxx_override is not None:
        cxx = find_executable(cxx_override)
        if cxx is None:
            raise Exception("C++ compiler not found: {}".format(cxx_override))

    toolchain_types: Mapping[ToolchainId, Type[Toolchain]] = {
        ToolchainId.CLANG: ClangLikeToolchain,
        ToolchainId.GCC: ClangLikeToolchain,
        ToolchainId.MSVC: MsvcToolchain
    }

    if cc is not None and cxx is not None:
        id = detect_compiler_from_exe(cc)
        return toolchain_types[id](
            id=id,
            architecture=architecture,
            binaries=ToolchainBinaries(cc=cc, cxx=cxx))

    system = platform.system()

    hints = []
    if system == "Windows":
        hints.append(("cl", "cl"))
    hints += (("gcc", "g++"),
              ("clang", "clang++"))

    for cc_hint, cxx_hint in hints:
        cc, cxx = map(find_executable, (cc_hint, cxx_hint))
        if cc is not None and cxx is not None:
            id = detect_compiler_from_exe(cc)
            return toolchain_types[id](
                id=id,
                architecture=architecture,
                binaries=ToolchainBinaries(cc=cc, cxx=cxx))

    raise Exception("Toolchain not found")


# def get_file_name_prefix_for_target_type(type: TargetType, system: str, compiler_id: CompilerId = None):
#    if type == TargetType.SHARED_LIBRARY:
#        if is_clang_like_compiler(compiler_id):
#            return "lib"
#        return {"Linux": "lib", "Darwin": "lib"}.get(system, "")
#    return ""


def detect_compiler_from_exe(exe_path: str) -> ToolchainId:
    system = platform.system()
    exe_name = os.path.basename(exe_path).lower()
    if system == "Windows":
        exe_name = exe_name.split(".exe")[0]
    # Detect based on file name.
    # if exe_name in ("clang", "clang++"):
    #    return CompilerId.CLANG
    # if exe_name in ("gcc", "g++"):
    #    return CompilerId.GCC
    if exe_name == "cl":
        return ToolchainId.MSVC
    # Detect based on macro definitions.
    # GCC/Clang parameters:
    #   -E                      Only run the preprocessor
    #   -dM                     Print macro definitions in -E mode instead of normal output
    #   -x <language>           Treat subsequent input files as having type <language>
    input_file_path = "nul" if system == "Windows" else "/dev/null"
    args = (exe_path, "-E", "-dM", "-x", "c", input_file_path)
    with subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE) as p:
        output, _ = p.communicate()
        output = output.decode("utf8")
        if "__clang__ " in output:
            return ToolchainId.CLANG
        # Other compilers may also define __GNUC__
        if "__GNUC__ " in output:
            return ToolchainId.GCC
    return None
