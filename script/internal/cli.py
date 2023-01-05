from internal.options import Options
from internal.utility import bool_to_str

from abc import abstractmethod
import argparse
from enum import Enum
import math
import os
import platform
import sys
from typing import Callable, IO, List, Sequence, Union


if sys.version_info >= (3, 8):
    from typing import get_args, get_type_hints
else:
    from typing_extensions import get_args, get_type_hints


class StrArgAction(argparse.Action):
    def __call__(self, parser, namespace, values, option_string=None):
        setattr(namespace, self.dest + "_set_explicitly", True)
        setattr(namespace, self.dest, values)


class BoolArgAction(argparse.Action):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.nargs = "?"
        self.const = True
        self.default = False
        self.choices = ("true", "false")

    def __call__(self, parser, namespace, values, option_string=None):
        if values is None:
            values = self.const
        elif type(values) == str:
            values = bool_to_str(values)
        setattr(namespace, self.dest + "_set_explicitly", True)
        setattr(namespace, self.dest, values)


class HelpFormatter(argparse.HelpFormatter):
    """
    Based on ArgumentDefaultsHelpFormatter and RawDescriptionHelpFormatter from
    argparse. Changes are as follows:
      * Allows unformatted text in the epilog.
      * Suppresses the default value in help text for boolean arguments.
      * Changes some text in the help text.
    """

    def _get_help_string(self, action):
        help = action.help
        if action.type != bool and type(action.const) != bool and action.const != None:
            if '%(default)' not in action.help:
                if action.default is not argparse.SUPPRESS:
                    defaulting_nargs = [
                        argparse.OPTIONAL, argparse.ZERO_OR_MORE]
                    if action.option_strings or action.nargs in defaulting_nargs:
                        help += ' (default: %(default)s)'
        return help

    def _fill_text(self, text, width, indent):
        return ''.join(indent + line for line in text.splitlines(keepends=True))

    def add_usage(self, usage, actions, groups, prefix=None):
        if prefix is None:
            prefix = "Usage: "
        super().add_usage(usage, actions, groups, prefix)


def parse_options(args, arg_parser_factory: Callable[[], argparse.ArgumentParser]):
    options = Options()
    arg_parser_factory().parse_args(args, namespace=options)
    for attr, option_type in get_type_hints(options).items():
        explicit = getattr(options, attr + "_set_explicitly", False)
        value = getattr(options, attr, None)
        value_type = get_args(option_type)[0]
        setattr(options, attr, option_type(
            None if value is None else value_type(value), explicit))
    return options


def is_terminal_interactive() -> bool:
    return sys.stdout.isatty()


def terminal_supports_escape() -> bool:
    # Windows: The Command shell does not support escape codes.
    #          The Windows Terminal supports escape codes but can we easily and reliably detect this?
    return is_terminal_interactive() and platform.system() != "Windows"


class PrintColor(Enum):
    RESET = 0
    FG_RED = 31
    FG_GREEN = 32
    FG_BLUE = 36
    BG_WHITE = 47


class PrintCommand:
    _callback: Callable[[IO], None]

    def __init__(self, callback: Callable[[IO], None]):
        self._callback = callback

    @abstractmethod
    def __call__(self, io: IO):
        self._callback(io)


class EscapeCodeCommand(PrintCommand):
    _escape_chars = "\033["

    def __init__(self, sequence: str):
        super().__init__(lambda io: io.write(self._escape_chars + sequence))


class PrintSequence:
    _sequence: Sequence[Union[PrintCommand, str]]
    _enable_commands: bool

    def __init__(self, sequence: Sequence[Union[PrintCommand, str]], with_commands: bool = False):
        self._sequence = sequence
        self._enable_commands = with_commands

    def print(self, to: IO = sys.stdout):
        for s in self._sequence:
            if isinstance(s, PrintCommand):
                if self._enable_commands:
                    s(to)
            else:
                to.write(s)


class PrintCommandFactory:
    def color(self, color: PrintColor):
        return EscapeCodeCommand(f"{color.value}m")

    def save_pos(self):
        return EscapeCodeCommand("s")

    def restore_pos(self):
        return EscapeCodeCommand("u")

    def insert_line(self):
        return EscapeCodeCommand("L")

    def delete_line(self):
        return EscapeCodeCommand("M")

    def move_y(self, y: int):
        return EscapeCodeCommand(f"{y}E")

    def flush(self):
        return PrintCommand(lambda io: io.flush())

    def write(self, s: str):
        return PrintCommand(lambda io: io.write(s))


class PrintSequenceBuilder:
    _escape_chars = "\033["
    _result: List[Union[PrintCommand, str]]
    _factory: PrintCommandFactory

    def __init__(self, factory: PrintCommandFactory = PrintCommandFactory()):
        self._result = []
        self._factory = factory

    def to_sequence(self, with_commands: bool = False) -> PrintSequence:
        return PrintSequence(self._result, with_commands=with_commands)

    def color(self, color: PrintColor):
        self._result.append(EscapeCodeCommand(f"{color.value}m"))

    def colored(self, start_color: PrintColor, s: str):
        self.color(start_color)
        self._result.append(s)
        self.color(PrintColor.RESET)

    def save_pos(self):
        self._result.append(self._factory.save_pos())

    def restore_pos(self):
        self._result.append(self._factory.restore_pos())

    def insert_line(self):
        self._result.append(self._factory.insert_line())

    def delete_line(self):
        self._result.append(self._factory.delete_line())

    def move_y(self, y: int):
        self._result.append(self._factory.move_y(y))

    def raw(self, s: str):
        self._result.append(s)

    def flush(self):
        self._result.append(PrintCommand(lambda io: io.flush()))

    def rich(self, *elements: Union[Callable[[IO], None], PrintCommand, str]):
        for element in elements:
            if isinstance(element, PrintCommand):
                self._result.append(element)
            elif callable(element):
                self._result.append(PrintCommand(element))
            elif isinstance(element, str):
                self._result.append(self._factory.write(element))

    def progress_bar(self, label: str, current: int, total: int, width: int = None):
        try:
            width = os.get_terminal_size().columns if width is None else width
        except:
            width = 0
        before_bar = "".join((
            label,
            ": ",
            (str(math.floor((current / float(total)) * 100)) + "%").ljust(4),
            "["
        ))
        after_bar = "]"
        other_length = len(before_bar) + len(after_bar)
        bar_width = width - other_length
        fill_count = math.floor((current / float(total)) * bar_width)
        self.insert_line()
        self.save_pos()
        self.move_y(9999)
        self.rich(
            before_bar,
            self._factory.color(PrintColor.BG_WHITE),
            " " * fill_count,
            self._factory.color(PrintColor.RESET),
            " " * (bar_width - fill_count),
            after_bar
        )
        self.flush()
        self.delete_line()
        self.restore_pos()
