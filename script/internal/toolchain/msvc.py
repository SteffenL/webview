from internal.build import BuildType, PropertyScope, RuntimeLinkMethod
from internal.common import Arch
from internal.utility import get_host_arch
from internal.target import Target, TargetType
from internal.toolchain.common import ArchiveParams, CompileParams, Language, LinkParams, Toolchain

import math
import os
import platform
import subprocess
from typing import List, Sequence


class MsvcToolchain(Toolchain):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def get_compile_exe(self, language: Language):
        if language is None:
            raise Exception("No language specified")
        if language == Language.CXX:
            exe = self._binaries.cxx
        elif language == Language.C:
            exe = self._binaries.cc
        else:
            raise Exception("Invalid language")
        return exe

    def get_archive_exe(self, language: Language):
        binaries = self.get_binaries()
        exe = binaries.ar
        if exe is None:
            exe = os.path.join(os.path.dirname(binaries.cc), "lib.exe")
        if exe is None:
            raise Exception("Binary not found: lib")
        return exe

    def get_link_exe(self, language: Language):
        return self.get_compile_exe(language)

    def get_archive_params(self, target: Target) -> ArchiveParams:
        input_paths: List[str] = []

        # Object files
        source_dir = target.get_workspace().get_source_dir()
        for source in target.get_sources():
            rel_source_path = os.path.relpath(source, source_dir)
            input_path = os.path.join(
                target.get_obj_dir(), rel_source_path + ".o")
            input_paths.append(input_path)

        output_dir = target.get_lib_dir()
        output_path = os.path.join(
            output_dir, target.get_output_file_name(extension=".a"))

        params = ArchiveParams()
        params.input_paths = input_paths
        params.output_path = output_path

        return params

    def get_compile_params(self, target: Target) -> Sequence[CompileParams]:
        system = platform.system()
        cflags: List[str] = []

        cflags += ("/nologo", "/utf-8")

        # Optimization
        build_type = target.get_build_type()
        if build_type == BuildType.DEBUG:
            cflags.append("/Od")
        elif build_type == BuildType.RELEASE:
            cflags.append("/O2")

        # Language standard
        standard = target.get_language_standard()
        if standard is not None:
            language = target.get_language()
            standard_str = "c" if language == Language.C else "c++" if language == Language.CXX else None
            standard_str += str(math.floor(standard / 100) % 100)
            cflags.append("/std:" + standard_str)

        # C++ exceptions
        if target.get_language() == Language.CXX:
            cflags.append("/EHsc")

        # Note: Architecture is dictated by toolchain environment.

        # Warnings

        # Definitions
        cflags += tuple("/D" + k if v is None else "/D{}={}".format(k, v)
                        for k, v in target.get_definitions(PropertyScope.INTERNAL).items())
        # Include directories
        cflags += tuple("/I{}".format(s)
                        for s in target.get_include_dirs(PropertyScope.INTERNAL))

        result = []
        for source in target.get_sources():
            params = CompileParams()
            params.cflags += cflags
            # Output path
            source_dir = target.get_workspace().get_source_dir()
            rel_source_path = os.path.relpath(source, source_dir)
            output_path = os.path.join(
                target.get_obj_dir(), rel_source_path + ".o")
            params.output_path = output_path
            # Sources
            params.input_path = source
            result.append(params)
        return result

    def get_link_params(self, target: Target) -> LinkParams:
        system = platform.system()
        ldflags: List[str] = []
        input_paths: List[str] = []


        # Note: Architecture is dictated by toolchain environment.

        # Object files
        source_dir = target.get_workspace().get_source_dir()
        for source in target.get_sources():
            # Output path
            rel_source_path = os.path.relpath(source, source_dir)
            input_path = os.path.join(
                target.get_obj_dir(), rel_source_path + ".o")
            input_paths.append(input_path)

        # Runtime linking
        # if target.get_type() in (TargetType.EXE, TargetType.SHARED_LIBRARY):
        #    if target.get_runtime_link_method() == RuntimeLinkMethod.STATIC:
        #        compile_command.append("-static")

        if target.get_type() in (TargetType.EXE, TargetType.SHARED_LIBRARY):
            # Link library directories
            ldflags += tuple("/L{}".format(s)
                             for s in target.get_library_dirs(PropertyScope.INTERNAL))
            # Link libraries
            for lib in target.get_link_libraries(PropertyScope.INTERNAL):
                if type(lib) == str:
                    if not ".lib" in lib:
                        lib += ".lib"
                    ldflags.append(lib)
                elif type(lib) == Target:
                    if lib.get_type() == TargetType.OBJECT:
                        input_paths.append(lib.get_output_file_path())
                    elif lib.get_type() in (TargetType.SHARED_LIBRARY, TargetType.STATIC_LIBRARY):
                        # Allow us to take advantage of rpath
                        ldflags.append("/LIBPATH:" + lib.get_lib_dir())
                        lib_name = lib.get_link_output_name()
                        if not ".lib" in lib_name:
                            lib_name += ".lib"
                        ldflags.append(lib_name)
                    #if lib.get_type() in (lib.get_type() == TargetType.OBJECT, TargetType.STATIC_LIBRARY):
                    #    if target.get_language() == Language.C and lib.get_language() == Language.CXX:
                    #        ldflags.append("-lc++" if system ==
                    #                       "Darwin" else "-lstdc++")
                else:
                    raise Exception("Invalid target type")

        target_type = target.get_type()
        output_dir = target.get_bin_dir(
        ) if target_type == TargetType.EXE else target.get_lib_dir()
        output_path = os.path.join(output_dir, target.get_output_file_name())

        params = LinkParams()
        params.ldflags = ldflags
        params.input_paths = input_paths
        params.output_path = output_path

        return params

    def _format_archive_params(self, target_type: TargetType, params: LinkParams) -> Sequence[str]:
        args: List[str] = []
        args += ("/nologo", "/OUT:" + params.output_path)
        args += params.input_paths
        return args

    def _format_compile_params(self, params: CompileParams, add_input: bool = False, add_output: bool = False) -> Sequence[str]:
        args: List[str] = []
        args += params.cflags
        if add_output:
            args += ("/c", "/Fo:" + params.output_path)
        if add_input:
            args.append(params.input_path)
        return args

    def _format_link_params(self, target_type: TargetType, params: LinkParams) -> Sequence[str]:
        args: List[str] = []
        args.append("/link")
        args += params.ldflags
        if target_type == TargetType.SHARED_LIBRARY:
            args += ("/DLL", "/Fe:" + params.output_path)
        args += params.input_paths
        return args

    def get_file_name_prefix(self, target_type: TargetType) -> str:
        return ""

    def get_file_name_extension(self, target_type: TargetType, system: str) -> str:
        if target_type == TargetType.OBJECT:
            return ".o"
        if target_type == TargetType.STATIC_LIBRARY:
            return ".lib"
        return super().get_file_name_extension(target_type, system)

    def get_darwin_target_platform(self, architecture: Arch):
        system = platform.system()
        # x86 is unsupported on macOS
        if system == "Darwin" and architecture != Arch.X86:
            macos_arch = {Arch.ARM64: "arm64",
                          Arch.X64: "x86_64"}[architecture]
            return macos_arch + "-apple-macos" + self._MACOS_TARGET_VERSION
        if system == "Linux":
            linux_arch = {Arch.ARM64: "arm64",
                          Arch.X64: "x86_64", Arch.X86: "i386"}[architecture]
            return linux_arch + "-linux"
        raise Exception(
            "Unsupported target system/architecture: {}/{}".format(system, architecture.value))


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
