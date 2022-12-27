from internal.build import Language, Phase, PropertyScope
from internal.context import Context
from internal.target import TargetType
from internal.utility import extract_file, download_file

import os
import platform


def fetch_mswebview2(version: str, destination_path: str):
    url = "https://www.nuget.org/api/v2/package/Microsoft.Web.WebView2/{}".format(
        version)
    with download_file(url) as file:
        extract_file(file, destination_path)


def should_fetch_mswebview2(context: Context, dep_dir: str):
    if platform.system() != "Windows":
        return False
    if not context.get_options().fetch_deps.get_value():
        return False
    if os.path.exists(dep_dir):
        return False
    return True


def register(context: Context):
    """Registers the MS WebView2 dependency."""

    arch = context.get_target_arch()
    version = context.get_options().mswebview2_version.get_value()
    root_dir_name = "mswebview2"
    root_dir = os.path.join(context.get_deps_dir(), root_dir_name, version)
    lib_dir = os.path.join(root_dir, "build", "native", arch.value.lower())

    target = context.add_target(
        TargetType.SHARED_LIBRARY, "mswebview2", Language.CXX)
    target.add_include_dirs(PropertyScope.EXTERNAL, os.path.join(
        root_dir, "build", "native", "include"))
    target.add_library_dirs(PropertyScope.EXTERNAL, lib_dir)
    target.set_output_name("WebView2Loader")
    target.set_output_dir(lib_dir)
    target.set_link_output_name("WebView2Loader.dll")

    context.add_task(Phase.PRE_COMPILE, "fetch_mswebview2",
                     lambda _: fetch_mswebview2(version, root_dir),
                     condition=lambda _: should_fetch_mswebview2(context, root_dir))
