from internal.common import Arch

from http.client import HTTPResponse
import platform
import shutil
import subprocess
from tempfile import TemporaryDirectory, TemporaryFile
from typing import IO, Union
from urllib.request import Request, urlopen
from zipfile import ZipFile


def bool_to_str(s: str):
    if is_true_string(s):
        return True
    if is_false_string(s):
        return False
    raise Exception("Cannot convert string to boolean value: " + s)


def download_file(url: str) -> IO[bytes]:
    request = Request(url, method="GET")
    with urlopen(request) as response:
        response: HTTPResponse
        temp_file = TemporaryFile()
        try:
            shutil.copyfileobj(response, temp_file)
            return temp_file
        except:
            temp_file.close()
            raise


def extract_file(file: Union[str, IO[bytes]], destination_path: str):
    zip_file = ZipFile(file)
    with TemporaryDirectory() as temp_dir:
        zip_file.extractall(temp_dir)
        shutil.move(temp_dir, destination_path)


def find_executable(name: str) -> Union[str, None]:
    name += ".exe" if platform.system() == "Windows" else ""
    which_exe = "where" if platform.system() == "Windows" else "which"
    with subprocess.Popen((which_exe, name), stdout=subprocess.PIPE, stderr=subprocess.PIPE) as p:
        output, _ = p.communicate()
        if p.returncode != 0 or output is None:
            return None
        output = output.decode("utf-8").strip().splitlines()[0]
        return output


def get_host_arch() -> Arch:
    arch = platform.machine().lower()
    if arch in ("amd64", "x86_64"):
        return Arch.X64
    if arch == ("i386", "x86"):
        return Arch.X86
    #if arch == "aarch64":
    #    return Arch.ARM64
    #if arch == "armv7l":
    #    return Arch.ARM32
    raise Exception("Unsupported host machine architecture.")


def is_false_string(s: str):
    return s.lower() in ("0", "false", "no")


def is_true_string(s: str):
    return s.lower() in ("1", "true", "yes")
