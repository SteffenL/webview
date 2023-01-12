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


class ClangLikeToolchain(Toolchain):
    _MACOS_TARGET_VERSION = "10.9"

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
            ext = ".exe" if platform.system() == "Windows" else ""
            exe = os.path.join(os.path.dirname(binaries.cc), "ar" + ext)
        if exe is None:
            raise Exception("Binary not found: ar")
        return exe

    def get_link_exe(self, language: Language):
        binaries = self.get_binaries()
        if language == Language.C:
            return binaries.cc
        if language == Language.CXX:
            return binaries.cxx
        raise NotImplementedError()

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

        pkgconfig_libs = target.get_pkgconfig_libs(PropertyScope.INTERNAL)
        if system == "Linux" and len(pkgconfig_libs) > 0:
            pkgconfig_cflags = subprocess.check_output(
                ("pkg-config", "--cflags", *pkgconfig_libs)).decode("utf-8").strip().split(" ")

        # Color output
        cflags.append("-fdiagnostics-color=always")

        # Optimization
        build_type = target.get_build_type()
        if build_type == BuildType.DEBUG:
            cflags += ("-Og", "-g")
        elif build_type == BuildType.RELEASE:
            cflags.append("-O2")

        # Language standard
        standard = target.get_language_standard()
        if standard is not None:
            language = target.get_language()
            standard_str = "c" if language == Language.C else "c++" if language == Language.CXX else None
            standard_str += str(math.floor(standard / 100) % 100)
            cflags.append("-std=" + standard_str)

        # Target platform
        arch = self.get_architecture()
        if system == "Darwin":
            cflags += ("-target", self.get_darwin_target_platform(arch))
        elif arch != Arch.NATIVE:
            if arch in (Arch.X64, Arch.X86):
                cflags.append({Arch.X64: "-m64", Arch.X86: "-m32"}[arch])

        # Warnings
        #args += ("-Wall", "-Wextra", "-pedantic")
        # if system == "Windows":
        #    # These warnings are emitted because of WebView2 so suppress them.
        #    args += ("-Wno-unknown-pragmas",
        #             "-Wno-unused-parameter", "-Wno-cast-function-type")

        # Shared libraries need PIC
        if target.get_type() == TargetType.SHARED_LIBRARY:
            cflags.append("-fPIC")

        # Definitions
        cflags += tuple("-D" + k if v is None else "-D{}={}".format(k, v)
                        for k, v in target.get_definitions(PropertyScope.INTERNAL).items())
        # Include directories
        cflags += tuple("-I{}".format(s)
                        for s in target.get_include_dirs(PropertyScope.INTERNAL))

        # pkgconfig flags
        if system == "Linux" and len(pkgconfig_libs) > 0:
            cflags += pkgconfig_cflags

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

        # Color output
        if platform.system() != "Windows":
            ldflags.append("-fdiagnostics-color=always")

        # Target platform
        arch = self.get_architecture()
        if system == "Darwin":
            ldflags += ("-target", self.get_darwin_target_platform(arch))
        elif arch != Arch.NATIVE:
            if arch in (Arch.X64, Arch.X86):
                ldflags.append({Arch.X64: "-m64", Arch.X86: "-m32"}[arch])

        # Object files
        source_dir = target.get_workspace().get_source_dir()
        for source in target.get_sources():
            # Output path
            rel_source_path = os.path.relpath(source, source_dir)
            input_path = os.path.join(
                target.get_obj_dir(), rel_source_path + ".o")
            input_paths.append(input_path)

        pkgconfig_libs = target.get_pkgconfig_libs(PropertyScope.INTERNAL)
        if system == "Linux" and len(pkgconfig_libs) > 0:
            if target.get_type() in (TargetType.EXE, TargetType.SHARED_LIBRARY):
                pkgconfig_ldflags = subprocess.check_output(
                    ("pkg-config", "--libs", *pkgconfig_libs)).decode("utf-8").strip().split(" ")

        if target.get_type() == TargetType.SHARED_LIBRARY:
            if system == "Darwin":
                # Make dylib for macOS
                ldflags.append("-dynamiclib")
            else:
                ldflags.append("-shared")
        elif target.get_type() == TargetType.STATIC_LIBRARY:
            # TODO: is this correct?
            ldflags.append("-static")

        # Runtime linking
        # if target.get_type() in (TargetType.EXE, TargetType.SHARED_LIBRARY):
        #    if target.get_runtime_link_method() == RuntimeLinkMethod.STATIC:
        #        compile_command.append("-static")

        # Frameworks
        if system == "Darwin":
            if target.get_type() in (TargetType.EXE, TargetType.SHARED_LIBRARY):
                for framework in target.get_macos_frameworks(PropertyScope.INTERNAL):
                    ldflags += ("-framework", framework)

        if target.get_type() in (TargetType.EXE, TargetType.SHARED_LIBRARY):
            # Link library directories
            ldflags += tuple("-L{}".format(s)
                             for s in target.get_library_dirs(PropertyScope.INTERNAL))
            # Link libraries
            for lib in target.get_link_libraries(PropertyScope.INTERNAL):
                if type(lib) == str:
                    ldflags.append("-l" + lib)
                elif type(lib) == Target:
                    if lib.get_type() == TargetType.OBJECT:
                        input_paths.append(lib.get_output_file_path())
                    elif lib.get_type() in (TargetType.SHARED_LIBRARY, TargetType.STATIC_LIBRARY):
                        # Allow us to take advantage of rpath
                        ldflags.append("-L" + lib.get_lib_dir())
                        ldflags.append("-l" + lib.get_link_output_name())
                    if lib.get_type() in (lib.get_type() == TargetType.OBJECT, TargetType.STATIC_LIBRARY):
                        if target.get_language() == Language.C and lib.get_language() == Language.CXX:
                            ldflags.append("-lc++" if system ==
                                           "Darwin" else "-lstdc++")
                else:
                    raise Exception("Invalid target type")

        # pkgconfig flags
        if system == "Linux" and len(pkgconfig_libs) > 0:
            if target.get_type() in (TargetType.EXE, TargetType.SHARED_LIBRARY):
                ldflags += pkgconfig_ldflags

        target_type = target.get_type()
        output_dir = target.get_bin_dir(
        ) if target_type == TargetType.EXE else target.get_lib_dir()
        output_path = os.path.join(output_dir, target.get_output_file_name())

        # rpath
        if system == "Linux" and target.get_type() in (TargetType.EXE, TargetType.SHARED_LIBRARY):
            # Same directory as executable
            ldflags.append("-Wl,-rpath=$ORIGIN")
            # Executable directory's sibling lib directory
            ldflags.append("-Wl,-rpath=$ORIGIN/../lib")
        elif system == "Darwin":
            if target.get_type() == TargetType.SHARED_LIBRARY:
                ldflags += ("-install_name", "@rpath/" +
                            target.get_output_file_name())
            elif target.get_type() == TargetType.EXE:
                # Bundled
                ldflags += ("-rpath", "@executable_path/../Frameworks")
                # Non-bundled
                ldflags += ("-rpath", "@executable_path")
                ldflags += ("-rpath", "@executable_path/../lib")

        params = LinkParams()
        params.ldflags = ldflags
        params.input_paths = input_paths
        params.output_path = output_path

        return params

    def _format_archive_params(self, target_type: TargetType, params: LinkParams) -> Sequence[str]:
        args: List[str] = []
        args += ("rcs", params.output_path)
        args += params.input_paths
        return args

    def _format_compile_params(self, params: CompileParams, add_input: bool = False, add_output: bool = False) -> Sequence[str]:
        args: List[str] = []
        args += params.cflags
        if add_output:
            args += ("-c", "-o", params.output_path)
        if add_input:
            args.append(params.input_path)
        return args

    def _format_link_params(self, target_type: TargetType, params: LinkParams) -> Sequence[str]:
        args: List[str] = []
        args += ("-o", params.output_path)
        args += params.input_paths
        args += params.ldflags
        return args

    def get_file_name_prefix(self, target_type: TargetType) -> str:
        if target_type in (TargetType.SHARED_LIBRARY, TargetType.STATIC_LIBRARY):
            return "lib"
        return ""

    def get_file_name_extension(self, target_type: TargetType, system: str) -> str:
        if target_type == TargetType.OBJECT:
            return ".o"
        if target_type == TargetType.STATIC_LIBRARY:
            return ".a"
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
