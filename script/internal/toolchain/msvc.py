from internal.build import BuildType, PropertyScope, RuntimeLinkType
from internal.common import Arch
from internal.utility import get_host_arch
from internal.target import Target, TargetType
from internal.toolchain.common import ArchiveParams, CompileParams, Language, LinkParams, Toolchain

import math
import os
import platform
import subprocess
from typing import List, Sequence

ARCH_TO_MSVC_COMPONENT_ARCH_MAP = {
    Arch.X64: "x86.x64",
    Arch.X86: "x86.x64",
    Arch.ARM64: "ARM64",
    Arch.ARM32: "ARM"
}


ARCH_TO_DEV_CMD_ARCH_MAP = {
    Arch.X64: "x64",
    Arch.X86: "x86",
    Arch.ARM64: "arm64",
    Arch.ARM32: "arm"
}


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
        binaries = self.get_binaries()
        exe = binaries.cc
        if exe is None:
            exe = os.path.join(os.path.dirname(binaries.cc), "link.exe")
        if exe is None:
            raise Exception("Binary not found: link")
        return exe

    def get_archive_params(self, target: Target) -> ArchiveParams:
        input_paths: List[str] = []

        # Object files
        source_dir = target.get_workspace().get_source_dir()
        for source in target.get_sources():
            rel_source_path = os.path.relpath(source, source_dir)
            input_path = os.path.join(
                target.get_obj_dir(), rel_source_path + ".obj")
            input_paths.append(input_path)

        output_dir = target.get_lib_dir()
        output_path = os.path.join(
            output_dir, target.get_output_file_name(extension=".lib"))

        params = ArchiveParams()
        params.input_paths = input_paths
        params.output_path = output_path

        return params

    def get_compile_params(self, target: Target) -> Sequence[CompileParams]:
        cflags: List[str] = []

        # "/options:strict"
        cflags += ("/nologo", "/utf-8")

        # Warnings
        cflags += tuple(param for param in target.get_warning_params(PropertyScope.INTERNAL))

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

        # Runtime linking
        runtime_link_param = get_target_msvc_runtime_link_param(target)
        print("runtime", runtime_link_param)
        if runtime_link_param is not None:
            cflags.append(runtime_link_param)

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
                target.get_obj_dir(), rel_source_path + ".obj")
            params.output_path = output_path
            # Sources
            params.input_path = source
            result.append(params)
        return result

    def get_link_params(self, target: Target) -> LinkParams:
        ldflags: List[str] = []
        input_paths: List[str] = []

        ldflags += ("/nologo", "/utf-8")

        # Warnings
        ldflags += tuple(param for param in target.get_warning_params(PropertyScope.INTERNAL))

        # Note: Architecture is dictated by toolchain environment.

        # Object files
        source_dir = target.get_workspace().get_source_dir()
        for source in target.get_sources():
            # Output path
            rel_source_path = os.path.relpath(source, source_dir)
            input_path = os.path.join(
                target.get_obj_dir(), rel_source_path + ".obj")
            input_paths.append(input_path)

        ldflags.append("/link")

        if target.get_type() == TargetType.SHARED_LIBRARY:
            ldflags.append("/DLL")

        if target.get_type() in (TargetType.EXE, TargetType.SHARED_LIBRARY):
            # Link library directories
            ldflags += tuple("/LIBPATH:{}".format(s)
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
                    # if lib.get_type() in (lib.get_type() == TargetType.OBJECT, TargetType.STATIC_LIBRARY):
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
        args.append("/nologo")
        args.append("/OUT:" + params.output_path)
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
        args += params.input_paths
        args += params.ldflags
        args.append("/OUT:" + params.output_path)
        return args

    def get_file_name_prefix(self, target_type: TargetType) -> str:
        return ""

    def get_file_name_extension(self, target_type: TargetType, system: str) -> str:
        if target_type == TargetType.OBJECT:
            return ".obj"
        if target_type == TargetType.STATIC_LIBRARY:
            return ".lib"
        return super().get_file_name_extension(target_type, system)


def find_msvc_dev_cmd(target_arch: Arch):
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
    msvc_arch_component_suffix = ARCH_TO_MSVC_COMPONENT_ARCH_MAP[target_arch]
    vswhere_args = (vswhere_path, "-latest", "-products", "*", "-requires",
                    "Microsoft.VisualStudio.Component.VC.Tools." + msvc_arch_component_suffix,
                    "-property", "installationPath")
    vs_dir = subprocess.check_output(vswhere_args, encoding="utf8").strip()
    vsdevcmd_path = os.path.join(vs_dir, "Common7", "Tools", "vsdevcmd.bat")
    if not os.path.exists(vsdevcmd_path):
        raise Exception("Unable to find vsdevcmd.bat")
    return vsdevcmd_path


def activate_msvc_toolchain(architecture: Arch):
    dev_cmd_target_arch = ARCH_TO_DEV_CMD_ARCH_MAP[architecture]
    dev_cmd_host_arch = ARCH_TO_DEV_CMD_ARCH_MAP[get_host_arch()]
    dev_cmd_path = find_msvc_dev_cmd(architecture)
    # Extract environment variables set by dev cmd.
    dev_cmd_args = ("cmd.exe", "/C", "call", dev_cmd_path, "-no_logo", "-arch=" +
                    dev_cmd_target_arch, "-host_arch=" + dev_cmd_host_arch, "&&", "set")
    dev_cmd_output = subprocess.check_output(
        dev_cmd_args, encoding="utf8").strip()
    dev_cmd_env = tuple(kv.split("=", 1)
                        for kv in dev_cmd_output.splitlines())
    # Temporarily update the current environment with the variables extracted from dev cmd.
    os.environ.update(dev_cmd_env)


def get_target_msvc_runtime_link_param(target: Target):
    runtime_type = target.get_runtime_link_method()
    build_type = target.get_build_type()
    if runtime_type == RuntimeLinkType.SHARED:
        if build_type == BuildType.DEBUG:
            return "/MDd"
        elif build_type == BuildType.RELEASE:
            return "/MD"
    elif runtime_type == RuntimeLinkType.STATIC:
        if build_type == BuildType.DEBUG:
            return "/MTd"
        elif build_type == BuildType.RELEASE:
            return "/MT"
    raise Exception(
        "Should not happen: Unable to determine MSVC runtime linking")
