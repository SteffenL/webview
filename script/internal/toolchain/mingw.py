from internal.common import Arch
from internal.toolchain.common import ToolchainId
from internal.utility import get_host_arch


import os
from typing import Callable


def find_mingw_path(hint: str, arch: Arch, toolchain: ToolchainId = None):
    # Order the combinations according to the likeliness of it being found on the system.
    combinations = (
        (ToolchainId.GCC, (Arch.ARM64, Arch.X64), (Arch.X64,), "mingw64"),
        (ToolchainId.GCC, (Arch.ARM64, Arch.X64, Arch.X86), (Arch.X86,), "mingw32"),
        (ToolchainId.CLANG, (Arch.ARM64, Arch.X64), (Arch.X64,), "clang64"),
        (ToolchainId.CLANG, (Arch.ARM64, Arch.X64, Arch.X86), (Arch.X86,), "clang32"),
        (ToolchainId.CLANG, (Arch.ARM64,), (Arch.ARM64,), "clangarm64"),
        (None, None, None, "mingw")
    )

    test_sub_paths = []
    # Prefer GCC
    test_exe_names = {ToolchainId.GCC: "g++.exe", ToolchainId.CLANG: "clang++.exe"}
    if toolchain is None:
        for exe in test_exe_names.values():
            test_sub_paths.append(os.path.join("bin", exe))
    else:
        test_sub_paths.append(os.path.join("bin", test_exe_names[toolchain]))

    host_arch = get_host_arch()

    for combination in combinations:
        c_id, c_host_arch, c_target_arch, c_path = combination
        c_path = os.path.join(hint, c_path)
        if c_host_arch is not None and not host_arch in c_host_arch:
            continue
        if c_target_arch is not None and not arch in c_target_arch:
            continue
        if toolchain is not None and c_id is not None and toolchain != c_id:
            continue
        for test_sub_path in test_sub_paths:
            if os.path.exists(os.path.join(c_path, test_sub_path)):
                return c_path
    return None


def find_any_mingw_path(arch: Arch, toolchain: ToolchainId = None):
    system_drive_root_dir = os.path.normpath(
        os.environ["SystemDrive"] + os.path.sep)
    program_data_dir = os.path.normpath(os.environ["ProgramData"])
    hints = (
        system_drive_root_dir,
        os.path.join(program_data_dir, "chocolatey",
                     "lib", "mingw", "tools", "install"),
        os.path.join(system_drive_root_dir, "msys64")
    )
    for hint in hints:
        mingw_path = find_mingw_path(hint, arch, toolchain)
        if mingw_path is not None:
            return os.path.join(mingw_path, "bin")
    return None


def activate_mingw_toolchain(arch: Arch, toolchain: ToolchainId = None):
    mingw_path = find_any_mingw_path(arch, toolchain)
    if mingw_path is None:
        raise Exception("Unable to find MinGW-w64")
    env_backup = tuple(tuple(item) for item in os.environ.items())
    # Temporarily update PATH environment variable in the current environment.
    os.environ["PATH"] = os.path.pathsep.join((mingw_path, os.environ["PATH"]))
