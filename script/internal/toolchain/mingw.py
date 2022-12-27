from internal.common import Arch


import os
from typing import Callable


def find_mingw_path(hint: str, arch: Arch):
    mingw64_path = os.path.join(hint, "mingw64")
    mingw32_path = os.path.join(hint, "mingw32")
    test_sub_path = os.path.join("bin", "g++.exe")
    if arch == Arch.X86:
        if os.path.exists(os.path.join(mingw32_path, test_sub_path)):
            return mingw32_path
    if arch in (Arch.X86, Arch.X64):
        if os.path.exists(os.path.join(mingw64_path, test_sub_path)):
            return mingw64_path
    return None


def find_any_mingw_path(arch: Arch):
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
        mingw_path = find_mingw_path(hint, arch)
        if mingw_path is not None:
            return os.path.join(mingw_path, "bin")
    return None


def activate_mingw_toolchain(arch: Arch,):
    mingw_path = find_any_mingw_path(arch)
    if mingw_path is None:
        raise Exception("Unable to find MinGW-w64")
    env_backup = tuple(tuple(item) for item in os.environ.items())
    # Temporarily update PATH environment variable in the current environment.
    os.environ["PATH"] = os.path.pathsep.join((mingw_path, os.environ["PATH"]))
