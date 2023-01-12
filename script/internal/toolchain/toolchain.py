from internal.common import Arch
from internal.target import TargetType
from internal.utility import find_executable
#from internal.toolchain.clang_like import ClangLikeCompiler, is_clang_like_compiler
from internal.toolchain.clang_like import ClangLikeToolchain
from internal.toolchain.common import Toolchain, ToolchainBinaries, ToolchainEnvironmentId, ToolchainId
from internal.toolchain.mingw import activate_mingw_toolchain
from internal.toolchain.msvc import activate_msvc_toolchain, MsvcToolchain

from dataclasses import dataclass
import os
import platform
import subprocess
from typing import Callable, Mapping, Tuple, Type


def activate_toolchain(id: ToolchainEnvironmentId, arch: Arch):
    system = platform.system()
    if system == "Windows":
        if id == ToolchainEnvironmentId.MSVC:
            activate_msvc_toolchain(arch)
            return
        if id == ToolchainEnvironmentId.MINGW:
            activate_mingw_toolchain(arch)
            return
    raise Exception("Invalid toolchain environment: " + id.value)


def detect_toolchain(architecture: Arch,
                     toolchain_prefix: str = None,
                     ar_override: str = None,
                     cc_override: str = None,
                     cxx_override: str = None,
                     ld_override: str = None) -> Toolchain:
    if any([(exe is None) != (ar_override is None) for exe in (cc_override, cxx_override, ld_override)]):
        raise Exception(
            "Either all toolchain binary overrides or none must be specified")

    toolchain_prefix = "" if toolchain_prefix is None else toolchain_prefix

    ar: str = None
    cc: str = None
    cxx: str = None
    ld: str = None

    if ar_override is not None:
        ar_override = toolchain_prefix + ar_override
        ar = find_executable(ar_override)
        if ar is None:
            raise Exception("Archiver not found: {}".format(ar_override))

    if cc_override is not None:
        cc_override = toolchain_prefix + cc_override
        cc = find_executable(cc_override)
        if cc is None:
            raise Exception("C compiler not found: {}".format(cc_override))

    if cxx_override is not None:
        cxx_override = toolchain_prefix + cxx_override
        cxx = find_executable(cxx_override)
        if cxx is None:
            raise Exception("C++ compiler not found: {}".format(cxx_override))

    if ld_override is not None:
        ld_override = toolchain_prefix + ld_override
        ld = find_executable(ld_override)
        if ld is None:
            raise Exception("Linker not found: {}".format(ld_override))

    toolchain_types: Mapping[ToolchainId, Type[Toolchain]] = {
        ToolchainId.CLANG: ClangLikeToolchain,
        ToolchainId.GCC: ClangLikeToolchain,
        ToolchainId.MSVC: MsvcToolchain
    }

    if all([exe is not None for exe in (ar, cc, cxx, ld)]):
        id = detect_compiler_from_exe(cc)
        if id is not None:
            return toolchain_types[id](
                id=id,
                architecture=architecture,
                binaries=ToolchainBinaries(ar=ar, cc=cc, cxx=cxx, ld=ld))

    system = platform.system()

    hints = []
    if system == "Windows":
        hints.append(("lib", "cl", "cl", "link"))
    hints += (("ar", "gcc", "g++", "ld"),
              ("ar", "clang", "clang++", "ld"))

    for chain in hints:
        chain = tuple(toolchain_prefix + exe for exe in chain)
        chain = tuple(map(find_executable, chain))
        if all([exe is not None for exe in chain]):
            ar, cc, cxx, ld = chain
            id = detect_compiler_from_exe(cc)
            if id is not None:
                return toolchain_types[id](
                    id=id,
                    architecture=architecture,
                    binaries=ToolchainBinaries(ar=ar, cc=cc, cxx=cxx, ld=ld))

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
