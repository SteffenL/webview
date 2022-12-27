from internal.context import Context
import targets.deps.mswebview2

import platform


def register(context: Context):
    """Registers all dependencies."""

    system = platform.system()
    if system == "Windows":
        targets.deps.mswebview2.register(context)
