from internal.build import BuildType, Language, LanguageStandard, PropertyScope, RuntimeLinkMethod

from typing import TYPE_CHECKING
if TYPE_CHECKING:
    from internal.workspace import Workspace

from enum import Enum
import os
import platform
import shutil
from typing import Callable, Iterable, List, Mapping, MutableMapping, MutableSequence, Sequence, Tuple, TypeVar, Union


T = TypeVar("T")


class TargetType(Enum):
    EXE = "executable"
    INTERFACE = "interface library"
    OBJECT = "object file"
    SHARED_LIBRARY = "shared library"
    STATIC_LIBRARY = "static_library"


class Target():
    _type: TargetType
    _name: str
    _language: Language
    _language_standards: MutableMapping[Language, Union[int, None]]
    _include_dirs: MutableMapping[PropertyScope, List[str]]
    _lib_dirs: MutableMapping[PropertyScope, List[str]]
    _link_libs: MutableMapping[PropertyScope, List[Union[str, "Target"]]]
    _sources: List[str]
    _definitions: MutableMapping[PropertyScope, MutableMapping[str, str]]
    _macos_frameworks: MutableMapping[PropertyScope, List[str]]
    _pkgconfig_libs: MutableMapping[PropertyScope, List[str]]
    _output_name: str
    _link_output_name: str
    _enabled: bool
    _runtime_link_method: RuntimeLinkMethod
    _build_type: BuildType
    _output_name_prefix: str
    _default_scope: Union[PropertyScope, Iterable[PropertyScope]]
    _workspace: "Workspace"
    _condition: Callable[[], bool]
    _bin_dir: str
    _lib_dir: str
    _obj_dir: str
    _uses_threads: MutableMapping[PropertyScope, bool]

    def __init__(self, workspace: "Workspace", type: TargetType, name: str, language: Language = None):
        self._type = type
        self._name = name
        self._language = language
        self._language_standards = dict((k, None) for k in Language)
        self._include_dirs = self._initialize_scoped([])
        self._lib_dirs = self._initialize_scoped([])
        self._link_libs = self._initialize_scoped([])
        self._sources = []
        self._definitions = self._initialize_scoped({})
        self._macos_frameworks = self._initialize_scoped([])
        self._pkgconfig_libs = self._initialize_scoped([])
        self._output_name = None
        self._link_output_name = None
        self._enabled = True
        self._runtime_link_method = RuntimeLinkMethod.SHARED
        self._build_type = None
        self._output_name_prefix = None
        self._default_scope = PropertyScope.EXTERNAL if type == TargetType.INTERFACE else PropertyScope.INTERNAL
        self._workspace = workspace
        self._condition = lambda: True
        self._bin_dir = None
        self._lib_dir = None
        self._obj_dir = None
        self._uses_threads = self._initialize_scoped(False)

    def __hash__(self) -> int:
        return hash((self._type, self._name))

    @staticmethod
    def _initialize_scoped(default: T) -> MutableMapping[PropertyScope, T]:
        return {PropertyScope.INTERNAL: default, PropertyScope.EXTERNAL: default}

    def get_type(self):
        return self._type

    def get_name(self):
        return self._name

    def get_language(self):
        return self._language

    def set_language(self, language: Language, standard: LanguageStandard = None):
        self._language = language
        if standard is not None:
            self.set_language_standard(standard)

    def get_language_standard(self, language: Language = None):
        if language is None:
            language = self._language
        if language is None:
            raise Exception("No language set")
        return self._language_standards[language]

    def set_language_standard(self, standard: LanguageStandard):
        self._language_standards[standard.get_language()
                                 ] = standard.get_standard()

    def get_include_dirs(self, scope: PropertyScope) -> Iterable[str]:
        return tuple(self._include_dirs[scope])

    def add_include_dirs(self, *dirs: str, scope: Union[PropertyScope, Iterable[PropertyScope]] = None):
        scope = self._normalize_property_scope_list(scope)
        for s in scope:
            self._include_dirs[s] += map(os.path.normpath, dirs)

    def get_library_dirs(self, scope: PropertyScope) -> Iterable[str]:
        return tuple(self._lib_dirs[scope])

    def add_library_dirs(self, *dirs: str, scope: Union[PropertyScope, Iterable[PropertyScope]] = None):
        scope = self._normalize_property_scope_list(scope)
        for s in scope:
            self._lib_dirs[s] += map(os.path.normpath, dirs)

    def get_link_libraries(self, scope: Union[PropertyScope, Iterable[PropertyScope]]) -> Iterable[Union[str, "Target"]]:
        scope = self._normalize_property_scope_list(scope)
        return tuple(b for a in scope for b in self._link_libs[a])

    def add_link_libraries(self, *libs: Union[str, "Target"], scope: Union[PropertyScope, Iterable[PropertyScope]] = None):
        if len(libs) == 0:
            return

        I = PropertyScope.INTERNAL
        E = PropertyScope.EXTERNAL
        scope = self._normalize_property_scope_list(scope)

        # Add the specified dependencies.
        for s in scope:
            self._link_libs[s] = list(dict.fromkeys(
                self._link_libs[s] + list(libs)))

        for lib in libs:
            if type(lib) == type(self):
                # Take the most recent language standard one from dependencies.
                for language in Language:
                    our_standard = self.get_language_standard(language)
                    if our_standard is None:
                        our_standard = 0
                    their_standard = lib.get_language_standard(language)
                    if their_standard is None:
                        their_standard = 0
                    if their_standard > our_standard:
                        self.set_language_standard(
                            LanguageStandard(language, their_standard))

                # Add internal and external pkg-config packages from dependency.
                self._pkgconfig_libs[I] = list(dict.fromkeys(
                    self._pkgconfig_libs[I] + lib._pkgconfig_libs[E]))
                self._pkgconfig_libs[E] = list(dict.fromkeys(
                    self._pkgconfig_libs[E] + lib._pkgconfig_libs[E]))

                # Add internal and external include directories from dependency.
                self._include_dirs[I] = list(dict.fromkeys(
                    self._include_dirs[I] + lib._include_dirs[E]))
                self._include_dirs[E] = list(dict.fromkeys(
                    self._include_dirs[E] + lib._include_dirs[E]))

                # Add internal and external link library directories from dependency.
                self._lib_dirs[I] = list(dict.fromkeys(
                    self._lib_dirs[I] + lib._lib_dirs[E]))
                self._lib_dirs[E] = list(dict.fromkeys(
                    self._lib_dirs[E] + lib._lib_dirs[E]))

                # Add internal and external link libraries from dependency.
                self._link_libs[I] = list(dict.fromkeys(
                    self._link_libs[I] + lib._link_libs[E]))
                self._link_libs[E] = list(dict.fromkeys(
                    self._link_libs[E] + lib._link_libs[E]))

                # Add internal and external definitions from dependency.
                for k, v in lib._definitions[E].items():
                    self.add_definition(k, value=v, scope=I)
                    self.add_definition(k, value=v, scope=E)

                # Add internal and external frameworks from dependency.
                self._macos_frameworks[I] = list(dict.fromkeys(
                    self._macos_frameworks[I] + lib._macos_frameworks[E]))
                self._macos_frameworks[E] = list(dict.fromkeys(
                    self._macos_frameworks[E] + lib._macos_frameworks[E]))

                # Use threads if library uses threads
                if lib._uses_threads[E]:
                    self._uses_threads[I] = True
                    self._uses_threads[E] = True

    def get_sources(self) -> Iterable[str]:
        return tuple(self._sources)

    def add_sources(self, *sources: str):
        normalized_sources = self._normalize_file_paths(sources)
        if self._language is None:
            self._language = self._detect_language_from_sources(
                normalized_sources)
        self._sources += normalized_sources

    def get_definitions(self, scope: PropertyScope) -> Mapping[str, str]:
        return dict(**self._definitions[scope])

    def add_definition(self, key: str, value: str = None, scope: Union[PropertyScope, Iterable[PropertyScope]] = None):
        scope = self._normalize_property_scope_list(scope)
        for s in scope:
            self._definitions[s][key] = value

    # def add_definitions(self, *definitions: Mapping[str, str]):
    #    self._definitions.update(definitions)

    def get_macos_frameworks(self, scope: PropertyScope) -> Iterable[str]:
        return tuple(self._macos_frameworks[scope])

    def add_macos_frameworks(self, *frameworks: str, scope: Union[PropertyScope, Iterable[PropertyScope]] = None):
        scope = self._normalize_property_scope_list(scope)
        for s in scope:
            self._macos_frameworks[s] += list(frameworks)

    def get_pkgconfig_libs(self, scope: PropertyScope) -> Iterable[str]:
        return tuple(self._pkgconfig_libs[scope])

    def add_pkgconfig_libs(self, *libs: str, scope: Union[PropertyScope, Iterable[PropertyScope]] = None):
        scope = self._normalize_property_scope_list(scope)
        for s in scope:
            self._pkgconfig_libs[s] += list(libs)

    def get_output_name(self):
        return self.get_name() if self._output_name is None else self._output_name

    def set_output_name(self, name: str):
        self._output_name = name

    def get_link_output_name(self):
        return self.get_output_name() if self._link_output_name is None else self._link_output_name

    def set_link_output_name(self, name: str):
        self._link_output_name = name

    def get_bin_dir(self):
        return self._workspace.get_bin_dir() if self._bin_dir is None else self._bin_dir

    def set_bin_dir(self, dir: str):
        self._bin_dir = dir

    def get_lib_dir(self):
        return self._workspace.get_lib_dir() if self._lib_dir is None else self._lib_dir

    def set_lib_dir(self, dir: str):
        self._lib_dir = dir

    def get_obj_dir(self):
        return self._workspace.get_obj_dir(self) if self._obj_dir is None else self._obj_dir

    def set_obj_dir(self, dir: str):
        self._obj_dir = dir

    def is_enabled(self):
        return self._enabled

    def set_enabled(self, enabled: bool):
        self._enabled = enabled

    def get_output_file_name(self, extension: str = None):
        type = self.get_type()
        toolchain = self._workspace.get_toolchain()
        prefix = self.get_output_name_prefix()
        prefix = toolchain.get_file_name_prefix(
            type) if prefix is None else prefix
        name = self.get_output_name()
        extension = toolchain.get_file_name_extension(
            type, platform.system()) if extension is None else extension
        return prefix + name + extension

    def get_output_dir(self):
        type = self.get_type()
        if type == TargetType.INTERFACE:
            return None
        if type == TargetType.EXE:
            dir = self.get_bin_dir()
        elif type == TargetType.OBJECT:
            dir = self.get_obj_dir()
        elif type in (TargetType.SHARED_LIBRARY, TargetType.STATIC_LIBRARY):
            dir = self.get_lib_dir()
        return dir

    def get_output_file_path(self):
        dir = self.get_output_dir()
        return os.path.join(dir, self.get_output_file_name())

    def get_runtime_link_method(self):
        return self._runtime_link_method

    def set_runtime_link(self, method: RuntimeLinkMethod):
        self._runtime_link_method = method

    def get_build_type(self):
        return self._workspace.get_build_type() if self._build_type is None else self._build_type

    def set_build_type(self, type: BuildType):
        self._build_type = type

    def _normalize_property_scope_list(self, scope: Union[PropertyScope, Iterable[PropertyScope]]) -> Iterable[PropertyScope]:
        if scope is None:
            scope = self._default_scope
        if type(scope) == PropertyScope:
            scope = [scope]
        return scope

    def get_output_name_prefix(self):
        return self._output_name_prefix

    def set_output_name_prefix(self, prefix: str):
        self._output_name_prefix = prefix

    def get_workspace(self):
        return self._workspace

    def set_condition(self, condition: Callable[[], bool]):
        self._condition = condition

    def is_condition_met(self):
        return self._condition()

    def is_using_threads(self, scope: PropertyScope):
        return self._uses_threads[scope]

    def set_uses_threads(self, scope: Union[PropertyScope, Iterable[PropertyScope]] = None):
        scope = self._normalize_property_scope_list(scope)
        for s in scope:
            self._uses_threads[s] = True

    def _detect_language_from_sources(self, sources: Iterable[str]) -> Language:
        extensions = {
            ".c": Language.C,
            ".cc": Language.CXX,
            ".cpp": Language.CXX,
            ".cxx": Language.CXX
        }
        for source in sources:
            root, ext = os.path.splitext(source)
            found = extensions.get(ext)
            if found is not None:
                return found
        return None

    def _normalize_file_paths(self, paths: Iterable[str]) -> Sequence[str]:
        normalized: MutableSequence[str] = []
        for p in paths:
            p = os.path.normpath(p)
            normalized.append(p if os.path.isabs(p) else os.path.join(
                self._workspace.get_source_dir(), p))
        return normalized
