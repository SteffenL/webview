from internal.build import BuildType, Language, PropertyScope, RuntimeLinkMethod
from internal.common import Arch
from internal.target import Target, TargetType
from internal.toolchain.common import ToolchainId, Toolchain
from internal.utility import get_host_arch


import os
import platform
import subprocess
from typing import Callable, Sequence

class MsvcToolchain(Toolchain):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)


"""
class MsvcCompiler(Compiler):
    def __init__(self, compiler_exe: str):
        super().__init__(CompilerId.MSVC, compiler_exe)

    def get_compile_params(self, target: Target, arch: Arch, source_dir: str, add_output: bool, add_sources: bool):
        compile_command = []

        compile_command += ("/nologo", "/utf-8")
        # Language standard
        standard = target.get_language_standard()
        if standard is not None:
            compile_command.append("/std:" + standard)
        # Note: Architecture is dictated by toolchain environment.
        # C++ exceptions
        if target.get_language() == Language.CXX:
            compile_command.append("/EHsc")
        # Warnings
        compile_command += ("/W4", "/wd4100")
        # Output directory
        if add_output is not None:
            compile_command.append(
                "/Fo:" + target.get_output_dir() + os.path.sep)
        # Definitions
        compile_command += tuple("/D" + k if v is None else "/D{}={}".format(k, v)
                                 for k, v in target.get_definitions(PropertyScope.INTERNAL).items())
        # Include directories
        compile_command += tuple("/I{}".format(s)
                                 for s in target.get_include_dirs(PropertyScope.INTERNAL))
        # Runtime linking
        if target.get_type() in (TargetType.EXE, TargetType.SHARED_LIBRARY):
            runtime_link_param = {
                RuntimeLinkMethod.STATIC: {
                    BuildType.RELEASE: "/MT",
                    BuildType.DEBUG: "/MTd"
                },
                RuntimeLinkMethod.SHARED: {
                    BuildType.RELEASE: "/MD",
                    BuildType.DEBUG: "/MDd"
                }
            }[target.get_runtime_link_method()][target.get_build_type()]
            compile_command.append(runtime_link_param)
        # Sources
        if add_sources:
            compile_command += tuple((s if os.path.isabs(s) else os.path.join(
                source_dir, s)) for s in target.get_sources())
        if target.get_type() == TargetType.OBJECT:
            compile_command.append("/c")
        elif target.get_type() in (TargetType.EXE, TargetType.SHARED_LIBRARY):
            compile_command.append("/link")
            if target.get_type() == TargetType.SHARED_LIBRARY:
                compile_command.append("/DLL")
            # Link library directories
            compile_command += tuple("/LIBPATH:" + s
                                     for s in target.get_library_dirs(PropertyScope.INTERNAL))
            # Link libraries
            for lib in target.get_link_libraries(PropertyScope.INTERNAL):
                if type(lib) == str:
                    lib_name = lib
                elif type(lib) == Target:
                    if not lib.get_type() in (TargetType.EXE, TargetType.SHARED_LIBRARY):
                        continue
                    lib_name = lib.get_link_output_name()
                else:
                    raise Exception("Invalid target type")
                if not ".lib" in lib_name:
                    lib_name += ".lib"
                compile_command.append(lib_name)
            # Output path
            if add_output:
                compile_command.append("/OUT:" + target.get_output_path())
        return compile_command
"""

def find_msvc_dev_cmd():
    python_arch_bits, _ = platform.architecture()
    if python_arch_bits == "64bit":
        pf_path_x64 = os.environ["ProgramFiles"]
        pf_path_x86 = os.environ["ProgramFiles(x86)"]
    elif python_arch_bits == "32bit":
        pf_path_x64 = os.environ["ProgramW6432"]
        pf_path_x86 = os.environ["ProgramFiles"]
    else:
        raise Exception("Unsupported architecture")
    vswhere_rel_path_parts = (
        "Microsoft Visual Studio", "Installer", "vswhere.exe")
    vswhere_path_x64 = os.path.join(pf_path_x64, *vswhere_rel_path_parts)
    vswhere_path_x86 = os.path.join(pf_path_x86, *vswhere_rel_path_parts)
    if os.path.exists(vswhere_path_x86):
        vswhere_path = vswhere_path_x86
    elif os.path.exists(vswhere_path_x64):
        vswhere_path = vswhere_path_x64
    else:
        raise Exception("Unable to find vswhere.exe")
    vswhere_args = (vswhere_path, "-latest", "-products", "*", "-requires",
                    "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
                    "-property", "installationPath")
    vs_dir = subprocess.check_output(vswhere_args).decode("utf8").strip()
    vsdevcmd_path = os.path.join(vs_dir, "Common7", "Tools", "vsdevcmd.bat")
    if not os.path.exists(vsdevcmd_path):
        raise Exception("Unable to find vsdevcmd.bat")
    return vsdevcmd_path


def activate_msvc_toolchain(arch: Arch):
    arch_to_dev_cmd_arch = {
        Arch.X64: "x64",
        Arch.X86: "x86"
    }
    dev_cmd_target_arch = arch_to_dev_cmd_arch[arch]
    dev_cmd_host_arch = arch_to_dev_cmd_arch[get_host_arch()]
    dev_cmd_path = find_msvc_dev_cmd()
    # Extract environment variables set by dev cmd.
    dev_cmd_args = ("cmd.exe", "/C", "call", dev_cmd_path, "-no_logo", "-arch=" +
                    dev_cmd_target_arch, "-host_arch=" + dev_cmd_host_arch, "&&", "set")
    dev_cmd_output = subprocess.check_output(
        dev_cmd_args).decode("utf8").strip()
    dev_cmd_env = tuple(kv.split("=", 1)
                        for kv in dev_cmd_output.splitlines())
    # Temporarily update the current environment with the variables extracted from dev cmd.
    os.environ.update(dev_cmd_env)
