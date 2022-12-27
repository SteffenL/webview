from internal.options import Options
from internal.utility import bool_to_str

import argparse
import sys
from typing import Callable

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
