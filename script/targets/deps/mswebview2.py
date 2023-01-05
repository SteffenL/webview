from internal.build import Language, LanguageStandards, PropertyScope
from internal.target import TargetType
from internal.task import Task, TaskPhase, TaskRunner
from internal.utility import extract_file, download_file
from internal.workspace import Workspace

import os
import platform


def register(task_runner: TaskRunner, workspace: Workspace):
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
    target.add_include_dirs(os.path.join(root_dir, "build", "native", "include"), scope=PropertyScope.EXTERNAL)
    target.add_library_dirs(lib_dir, scope=PropertyScope.EXTERNAL)
    target.set_output_name("WebView2Loader")
    target.set_bin_dir(lib_dir)
    target.set_lib_dir(lib_dir)
    target.set_link_output_name("WebView2Loader.dll")

    def fetch(task: Task):
        url = "https://www.nuget.org/api/v2/package/Microsoft.Web.WebView2/{}".format(
            version)
        with download_file(url) as file:
            extract_file(file, root_dir)

    def should_fetch(task: Task):
        if platform.system() != "Windows":
            return False
        if not workspace.get_options().fetch_deps.get_value():
            return False
        if os.path.exists(root_dir):
            return False
        return True

    fetch_tasks = task_runner.create_task_collection(
        TaskPhase.FETCH, concurrent=True)
    fetch_tasks.add_task(Task(fetch,
                              description="Fetch MS WebView2",
                              condition=should_fetch))
