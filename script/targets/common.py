from internal.build import Language, RuntimeLinkMethod
from internal.context import Context
from internal.target import Target

import platform


def apply_common_target_configuration(target: Target, context: Context):
    system = platform.system()
    lang = target.get_language()
    c_standard = "c99"
    cxx_standard = "c++17" if system == "Windows" else "c++11"
    standard = c_standard if lang == Language.C else cxx_standard if lang == Language.CXX else None
    target.set_language_standard(standard)
    # target.add_include_dirs(context.get_source_dir())
    # target.add_library_dirs(context.get_build_arch_dir())
    # if system == "Darwin":
    #    target.add_macos_frameworks("WebKit")
    # elif system == "Linux":
    #    target.add_pkgconfig_libs("gtk+-3.0", "webkit2gtk-4.0")
    if system == "Windows":
        target.set_runtime_link(RuntimeLinkMethod.STATIC)
    return target
