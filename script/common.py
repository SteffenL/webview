import ctypes
from dataclasses import dataclass
from enum import Enum
import inspect
import os
import platform
import re
import shutil
import subprocess
import sys
from typing import Callable, Mapping, Sequence

cmd_func_name_pattern = re.compile(r"^cmd_(.*)")

class OptionSpec:
    name: str
    help: str = None

    def __init__(self, name: str, help: str = None):
        self.name = name
        self.help = help

class CommandSpec:
    name: str
    func: Callable
    help: str = None
    options: Mapping[str, OptionSpec]

    def __init__(self, name: str, func: Callable, help: str = None, options: Sequence[OptionSpec] = ()):
        self.name = name
        self.func = func
        self.help = help
        self.options = dict((opt.name, opt) for opt in options)

class ProgramSpec:
    name: str
    description: str
    commands: Mapping[str, CommandSpec]

    def __init__(self, name: str, description: str, commands: Sequence[CommandSpec] = ()):
        self.name = name
        self.description = description
        self.commands = dict((cmd.name, cmd) for cmd in commands)

@dataclass
class CommandDocstring:
    brief: str = None
    parameters: Mapping[str, str] = None

def info(s: str):
    sys.stdout.write(f"{s}\n")
    sys.stdout.flush()

def error(s: str):
    sys.stdout.write(f"Error: {s}\n")
    sys.stdout.flush()

def parse_command_docstring(doc: str):
    """Simple and stupid command function docstring parser for our specific use case."""
    lines = tuple(s.strip() for s in doc.strip().splitlines())
    brief = lines[0]
    parameters = dict()
    option_pattern = re.compile(r"^[*-]*\s*(\w+)\s*--\s*(.*)")
    for line in lines:
        m = re.match(option_pattern, line)
        if m:
            parameters[m.group(1)] = m.group(2)
            continue
    return CommandDocstring(brief, parameters)

def create_command_spec(func: Callable):
    # Extract command name from function name and replace underscores with colons.
    name = ":".join(re.match(cmd_func_name_pattern, func.__name__).group(1).split("_"))
    doc = parse_command_docstring(func.__doc__)
    if not doc.brief:
        raise Exception(f"Command function \"{func.__name__}\" does not specify a brief description")
    doc_params = set(doc.parameters.keys())
    func_params = set(func.__annotations__.keys())
    mismatched_params = doc_params.difference(func_params)
    if len(mismatched_params) > 0:
        mismatched_params = ", ".join(mismatched_params)
        raise Exception(f"Parameters in docstring of \"{func.__name__}\" do not match function parameters: {mismatched_params}")
    return CommandSpec(
        name,
        func=func,
        help=doc.brief,
        options=tuple(OptionSpec(k, v) for k, v in doc.parameters.items()))

def create_program_spec(name: str, description: str, commands: Sequence[Callable]):
    return ProgramSpec(
        name,
        description=description,
        commands=tuple(create_command_spec(func) for func in commands))

def print_table(table: Sequence[Sequence[str]]):
    gap = 2
    col_widths: Mapping[int, int] = dict()
    for row in table:
        for col_index, col in enumerate(row):
            if len(col) > col_widths.get(col_index, 0):
                col_widths[col_index] = len(col)
    for row in table:
        line = ""
        for col_index, col in enumerate(row):
            w = (col_widths[col_index] + gap) if col_index + 1 < len(row) else 0
            line += col.ljust(w, " ")
        info(line)

def print_help(spec: ProgramSpec, for_cmd: str = None):
    if for_cmd is None:
        info(spec.description)
        info(f"Usage: {spec.name} COMMAND... [--help]")
        if len(spec.commands) > 0:
            info("Commands:")
            table = []
            for command in spec.commands.values():
                command_help = parse_command_docstring(command.func.__doc__)
                table.append((f"  {command.name}", command_help.brief))
            print_table(table)
        return
    command = spec.commands[for_cmd]
    command_help = parse_command_docstring(command.func.__doc__)
    info(command_help.brief)
    usage = f"Usage: {spec.name} {for_cmd}"
    if len(command_help.parameters) > 0:
        usage += " [OPTION...]"
    info(usage)
    if len(command_help.parameters) > 0:
        info("Options:")
        sig =  inspect.signature(command.func)
        table = []
        for k, v in command_help.parameters.items():
            param = sig.parameters[k.replace("-", "_")]
            k = k.replace("_", "-")
            if param.annotation == bool:
                table.append((f"  --{k}", v))
            elif issubclass(param.annotation, Enum):
                enum_values = tuple(x.value for x in param.annotation)
                enum_values = "|".join(enum_values)
                enum_values = f"<{enum_values}>"
                table.append((f"  --{k}={enum_values}", v))
            else:
                table.append((f"  --{k}={param.annotation.__name__}", v))
        print_table(table)

def create_arg_groups(args):
    commands = []
    arg_groups = dict()
    arg_groups[None] = dict()
    cmd = None
    for arg in args:
        if arg.startswith("-"):
            kv = arg.split("=", maxsplit=1)
            k = kv[0].lstrip("-").replace("-", "_")
            if k:
                v = kv[1] if len(kv) > 1 else True
                arg_groups[cmd][k] = v
        else:
            cmd = arg
            arg_groups[cmd] = dict()
            commands.append(cmd)
    return (commands, arg_groups)

def parse_args(args, spec: ProgramSpec):
    command_names, arg_groups = create_arg_groups(args)
    # Print default help text if no command was specified.
    if len(command_names) == 0:
        print_help(spec, None)
        sys.exit(1)
    if arg_groups[None].get("help"):
        print_help(spec, None)
        sys.exit(0)
    # Preprocesing of commands and options:
    # - Print help text before executing any command.
    # - Check if command function is callable with specified options.
    for command_name in command_names:
        if not command_name in spec.commands:
            error(f"Unknown command \"{command_name}\".", file = sys.stderr)
            sys.exit(1)
        cmd_spec_item = spec.commands[command_name]
        options = arg_groups[command_name]
        # Print command-specific help text.
        if options.get("help"):
            print_help(spec, command_name)
            sys.exit(0)
        option_names = options.keys()
        sig = inspect.signature(cmd_spec_item.func)
        # Check that the given options are present as parameters of the command function.
        for option_name in option_names:
            if not option_name in sig.parameters:
                error(f"Unknown option \"{option_name}\" passed to command \"{command_name}\".", file = sys.stderr)
                sys.exit(1)
        # Make sure that we can call the command function with the given options.
        bound = sig.bind(**options)
        for option_name in option_names:
            parameter = sig.parameters[option_name]
            value = bound.arguments[option_name]
            # If the value has the expected type then all is good.
            if isinstance(value, parameter.annotation):
                continue
            # The value is expected to be a string.
            if not isinstance(value, str):
                error(f"Invalid type for option \"{option_name}\" passed to command \"{command_name}\".", file = sys.stderr)
                sys.exit(1)
            # Convert option value to expected type. We need to handle each case here as needed.
            if parameter.annotation == bool:
                is_true = value.upper() in ("1", "Y", "YES", "TRUE", "ON")
                is_false = value.upper() in ("0", "N", "NO", "FALSE", "OFF")
                if is_true or is_false:
                    options[option_name] = is_true
                    continue
            elif parameter.annotation in (float, int, str) or issubclass(parameter.annotation, Enum):
                options[option_name] = parameter.annotation(value)
                continue
            else:
                error(f"Unknown type for option \"{option_name}\" passed to command \"{command_name}\".", file = sys.stderr)
                sys.exit(1)
            error(f"Invalid value \"{value}\" for option \"{option_name}\" passed to command \"{command_name}\".", file = sys.stderr)
            sys.exit(1)
    # Call command functions.
    for command_name in command_names:
        cmd_spec_item = spec.commands[command_name]
        options = arg_groups[command_name]
        if cmd_spec_item.func:
            cmd_spec_item.func(**options)

def load_cmake_cache(file_path: str):
    with open(file_path, "r", encoding="utf-8") as f:
        items = f.readlines()
    items = filter(lambda s: not s.startswith("#") and not s.startswith("/") and len(s.strip()) > 0, items)
    items = tuple(item.strip().split("=") for item in items)
    items = map(lambda item: (item[0].split(":")[0].strip(), item[1].strip()), items)
    items = dict(items)
    return items

def load_cmake_cache_from_dir(dir_path: str):
    return load_cmake_cache(os.path.join(dir_path, "CMakeCache.txt"))

def has_cmake_cache(build_dir: str):
    return os.path.isfile(os.path.join(build_dir, "CMakeCache.txt"))

class ShellId(Enum):
    SH = "sh"
    PWSH = "pwsh"
    CMD = "cmd"

def make_shell_var_name(name: str, shell: ShellId):
    if shell == ShellId.SH or shell == ShellId.PWSH:
        return "${" + name + "}"
    elif shell == ShellId.CMD:
        return f"%{name}%"
    else:
        raise Exception(f"Invalid shell: {shell}")

def make_shell_env_var_name(name: str, shell: ShellId):
    name = f"Env:{name}" if shell == ShellId.PWSH else name
    return make_shell_var_name(name, shell)

def escape_shell_string(s: str, shell: ShellId):
    if shell == ShellId.SH:
        return s.replace("\\", "\\\\").replace("$", "\\$")
    elif shell == ShellId.PWSH:
        return s.replace("\"", "\"\"").replace("`", "``")
    elif shell == ShellId.CMD:
        return s.replace("^", "^^").replace("\"", "^\"").replace("%", "^%").replace("!", "^!")
    raise Exception(f"Invalid shell: {shell}")

def make_shell_export_var_cmd(key: str, value: str, append_to: str, shell: ShellId):
    value = escape_shell_string(value, shell)
    existing = make_shell_env_var_name(append_to, shell) + os.pathsep
    if shell == ShellId.SH:
        return f"export {key}=\"{existing}{value}\""
    elif shell == ShellId.PWSH:
        return f"$Env:{key} = \"{existing}{value}\""
    elif shell == ShellId.CMD:
        return f"set \"{key}={existing}{value}\""
    else:
        raise Exception(f"Invalid shell: {shell}")

def detect_current_shell() -> ShellId:
    parent_pid = os.getppid()
    if platform.system() == "Windows":
        PROCESS_QUERY_LIMITED_INFORMATION = 0x1000
        process = ctypes.windll.kernel32.OpenProcess(ctypes.c_uint(PROCESS_QUERY_LIMITED_INFORMATION), ctypes.c_uint(False), ctypes.c_uint(parent_pid))
        if not process:
            raise Exception("Failed to open parent process")
        exe_path = ctypes.create_unicode_buffer(255)
        exe_path_length = ctypes.c_uint(len(exe_path))
        ok = ctypes.windll.kernel32.QueryFullProcessImageNameW(ctypes.c_void_p(process), ctypes.c_uint(0), exe_path, ctypes.byref(exe_path_length))
        ctypes.windll.kernel32.CloseHandle(process)
        if not ok:
            raise Exception("Failed to get exe path from parent process")
        exe_path_str = exe_path.value[0:exe_path_length.value]
        exe_name = os.path.basename(exe_path_str).lower()
        if exe_name in ("powershell.exe", "pwsh.exe"):
            return ShellId.PWSH
        if exe_name == "cmd.exe":
            return ShellId.CMD
        if exe_name in ("bash.exe", "sh.exe"):
            return ShellId.SH
        return None
    # TODO: Detect shell on non-Windows OS
    return ShellId.SH

def join_paths(env):
    return os.pathsep.join(env)

def is_windows():
    return platform.system() == "Windows"

def find_exe(name):
    if is_windows():
        name = name + f"{os.extsep}exe"
    return shutil.which(name)

def find_vswhere():
    parent_dir_hints = (
        os.environ["ProgramFiles"],
        os.environ["ProgramFiles(x86)"]
    )
    for parent_dir_hint in parent_dir_hints:
        vswhere_path = os.path.join(parent_dir_hint, "Microsoft Visual Studio", "Installer", "vswhere.exe")
        if os.path.isfile(vswhere_path):
            return vswhere_path
    return None

def find_vs_install_path():
    vswhere_path = find_vswhere()
    if not vswhere_path:
        return None
    vs_install_path = subprocess.check_output((
        vswhere_path,
        "-nologo",
        "-utf8",
        "-latest",
        "-format",
        "value",
        "-property",
        "installationPath"
    ), encoding="utf-8").strip()
    return vs_install_path

def find_cmake():
    cmake_path = find_exe("cmake")
    if cmake_path:
        return cmake_path
    if not is_windows():
        return None
    vs_install_path = find_vs_install_path()
    cmake_path = os.path.join(vs_install_path, "Common7", "IDE", "CommonExtensions", "Microsoft", "CMake", "CMake", "bin", "cmake.exe")
    if os.path.isfile(cmake_path):
        return cmake_path
    return None

def find_ninja():
    ninja_path = find_exe("ninja")
    if ninja_path:
        return ninja_path
    if not is_windows():
        return None
    vs_install_path = find_vs_install_path()
    ninja_path = os.path.join(vs_install_path, "Common7", "IDE", "CommonExtensions", "Microsoft", "CMake", "Ninja", "ninja.exe")
    if os.path.isfile(ninja_path):
        return ninja_path
    return None

def prepend_to_path_env(s: str):
    os.environ["PATH"] = os.pathsep.join([s] + os.environ["PATH"].split(os.pathsep))

def prepend_tool_to_path_env(find_fn: Callable):
    tool_path = find_fn()
    if tool_path:
        prepend_to_path_env(os.path.dirname(tool_path))

def prepend_tools_to_path_env(*find_fns: Sequence[Callable]):
    for find_fn in find_fns:
        prepend_tool_to_path_env(find_fn)
