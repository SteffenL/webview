from internal.build import Language, LanguageStandards, PropertyScope
from internal.target import TargetType
from internal.task import Task, TaskRunner
from internal.utility import extract_file, download_file
from internal.workspace import Workspace

import os
import platform


def fetch_mswebview2(arg):
    version: str
    destination_path: str
    version, destination_path = arg
    url = "https://www.nuget.org/api/v2/package/Microsoft.Web.WebView2/{}".format(
        version)
    with download_file(url) as file:
        extract_file(file, destination_path)


def should_fetch_mswebview2(workspace: Workspace, dep_dir: str):
    if platform.system() != "Windows":
        return False
    if not workspace.get_options().fetch_deps.get_value():
        return False
    if os.path.exists(dep_dir):
        return False
    return True


def register(workspace: Workspace):
    """Registers the MS WebView2 dependency."""

    toolchain = workspace.get_toolchain()
    arch = toolchain.get_architecture()
    version = workspace.get_options().mswebview2_version.get_value()
    root_dir_name = "mswebview2"
    root_dir = os.path.join(workspace.get_build_root_dir(),
                            ".deps", root_dir_name, version)
    lib_dir = os.path.join(root_dir, "build", "native", arch.value.lower())

    target = workspace.add_target(
        TargetType.SHARED_LIBRARY, "mswebview2")
    target.set_language(Language.CXX, standard=LanguageStandards.CXX17)
    target.add_include_dirs(PropertyScope.EXTERNAL, os.path.join(
        root_dir, "build", "native", "include"))
    target.add_library_dirs(PropertyScope.EXTERNAL, lib_dir)
    target.set_output_name("WebView2Loader")
    target.set_bin_dir(lib_dir)
    target.set_lib_dir(lib_dir)
    target.set_link_output_name("WebView2Loader.dll")

    """tasks = task_runner.create_task_collection()

    tasks.add_task(Task(fetch_mswebview2,
                        arg=(version, root_dir),
                        description="Fetch MS WebView2",
                        condition=lambda *_: should_fetch_mswebview2(
                            workspace, root_dir)))"""
