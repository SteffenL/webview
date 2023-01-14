from internal.common import Arch
from internal.utility import find_executable
from internal.toolchain.gcc_like import arch_to_gcc_machine, gcc_get_machine, GccLikeToolchain
from internal.toolchain.common import Toolchain, ToolchainBinaries, ToolchainEnvironmentId, ToolchainId
from internal.toolchain.mingw import activate_mingw_toolchain
from internal.toolchain.msvc import activate_msvc_toolchain, MsvcToolchain

import os
import platform
import subprocess
from typing import Mapping, Sequence, Type


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
                     whitelist: Sequence[ToolchainId] = None,
                     toolchain_prefix: str = None,
                     ar_override: str = None,
                     cc_override: str = None,
                     cxx_override: str = None,
                     ld_override: str = None) -> Toolchain:
    if any([(exe is None) != (ar_override is None) for exe in (cc_override, cxx_override, ld_override)]):
        raise Exception(
            "Either all toolchain binary overrides or none must be specified")

    whitelist = None if whitelist is None else set(whitelist)
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
        ToolchainId.CLANG: GccLikeToolchain,
        ToolchainId.GCC: GccLikeToolchain,
        ToolchainId.MSVC: MsvcToolchain
    }

    if all([exe is not None for exe in (ar, cc, cxx, ld)]):
        id = detect_compiler_from_exe(cc)
        if id is not None:
            if whitelist is not None and not id in whitelist:
                raise Exception(f"Toolchain not allowed: {id.value}")
            return toolchain_types[id](
                id=id,
                architecture=architecture,
                binaries=ToolchainBinaries(ar=ar, cc=cc, cxx=cxx, ld=ld))

    system = platform.system()

    all_chain_hints = []
    if system == "Darwin":
        all_chain_hints.append([["ar"], ["clang"], ["clang++"], ["ld64.lld"]])
        all_chain_hints.append([["ar"], ["gcc"], ["g++"], ["ld"]])
    elif system == "Linux":
        all_chain_hints.append([["ar"], ["gcc"], ["g++"], ["gold", "ld"]])
        all_chain_hints.append(
            [["llvm-ar", "ar"], ["clang"], ["clang++"], ["ld.lld", "ld"]])
    elif system == "Windows":
        all_chain_hints.append([["lib"], ["cl"], ["cl"], ["link"]])
        all_chain_hints.append([["ar"], ["gcc"], ["g++"], ["ld"]])
        all_chain_hints.append(
            [["llvm-ar", "ar"], ["clang"], ["clang++"], ["ld.lld", "ld"]])

    for chain_hints in all_chain_hints:
        found = []
        for exe_hints_index, exe_hints in enumerate(chain_hints):
            for exe_hint in exe_hints:
                exe = find_executable(toolchain_prefix + exe_hint)
                if exe:
                    found.append(exe)
                    break
                # Only try non-prefixed exe name if archiver and linker
                if exe_hints_index in (0, 3):
                    exe = find_executable(exe_hint)
                    if exe:
                        found.append(exe)
                        break
        if len(found) != len(chain_hints):
            continue
        if any([exe is None for exe in found]):
            continue
        ar, cc, cxx, ld = found
        id = detect_compiler_from_exe(cc)
        if id is not None:
            if whitelist is not None and not id in whitelist:
                continue
            if id != ToolchainId.MSVC:
                arch_as_gcc_machine = arch_to_gcc_machine(architecture)
                exe_gcc_machine = gcc_get_machine(cc)
                if not exe_gcc_machine in arch_as_gcc_machine:
                    raise Exception("Expected machine reported by executable ({}) to be one of [{}] but it was {}".format(
                        cc, ", ".join(arch_as_gcc_machine), exe_gcc_machine
                    ))
            return toolchain_types[id](
                id=id,
                architecture=architecture,
                binaries=ToolchainBinaries(ar=ar, cc=cc, cxx=cxx, ld=ld))

    raise Exception("Toolchain not found")


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
    with subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, encoding="utf8") as p:
        output, _ = p.communicate()
        if "__clang__ " in output:
            return ToolchainId.CLANG
        # Other compilers may also define __GNUC__
        if "__GNUC__ " in output:
            return ToolchainId.GCC
    return None
