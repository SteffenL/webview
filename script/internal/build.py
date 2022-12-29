from enum import Enum
import os
from typing import Iterable, List, Sequence, Tuple


class PropertyScope(Enum):
    INTERNAL = "internal"
    EXTERNAL = "external"


class PropertyScopes:
    PUBLIC = (PropertyScope.INTERNAL, PropertyScope.EXTERNAL)


class Language(Enum):
    C = "C"
    C_LIKE = "C-like"
    CXX = "C++",
    GO = "Go"

class LanguageStandard:
    _language: Language
    _standard: int

    def __init__(self, language: Language, standard: int) -> None:
        self._language = language
        self._standard = standard

    def get_language(self):
        return self._language

    def get_standard(self):
        return self._standard

class LanguageStandards:
    C99 = LanguageStandard(Language.C, 199901)
    C11 = LanguageStandard(Language.C, 201112)
    C17 = LanguageStandard(Language.C, 201710)
    CXX11 = LanguageStandard(Language.CXX, 201103)
    CXX14 = LanguageStandard(Language.CXX, 201402)
    CXX17 = LanguageStandard(Language.CXX, 201703)
    CXX20 = LanguageStandard(Language.CXX, 202002)


class RuntimeLinkMethod(Enum):
    STATIC = "static",
    SHARED = "shared"


class BuildType(Enum):
    DEBUG = "debug"
    RELEASE = "release"


class FileType(Enum):
    UNKNOWN = 0
    C_LIKE_HEADER = 1
    C_SOURCE = 2
    CXX_HEADER = 3
    CXX_SOURCE = 4
    GO = 5


file_ext_to_file_type_mapping = {
    ".h": FileType.C_LIKE_HEADER,
    ".hpp": FileType.CXX_HEADER,
    ".hxx": FileType.CXX_HEADER,
    ".c": FileType.C_SOURCE,
    ".cc": FileType.CXX_SOURCE,
    ".cpp": FileType.CXX_SOURCE,
    ".cxx": FileType.CXX_SOURCE,
    ".go": FileType.GO
}


file_type_to_language_mapping = {
    FileType.C_LIKE_HEADER: Language.C_LIKE,
    FileType.C_SOURCE: Language.C,
    FileType.CXX_HEADER: Language.CXX,
    FileType.CXX_SOURCE: Language.CXX,
    FileType.GO: Language.GO
}


def get_language_from_file_type(file_type: FileType):
    return file_type_to_language_mapping[file_type]


def find_sources(source_dir: str, file_types: Iterable[FileType], relative_to: str = None) -> Sequence[Tuple[FileType, str]]:
    all_files: List[Tuple[FileType, str]] = []
    for root, dirs, files in os.walk(source_dir):
        for file in files:
            _, ext = os.path.splitext(file)
            file_type = file_ext_to_file_type_mapping.get(ext.lower())
            if file_type is not None and file_type in file_types:
                item = (file_type, os.path.relpath(
                    os.path.join(root, file), source_dir if relative_to is None else relative_to))
                all_files.append(item)
    return all_files


def find_c_like_source_files(source_dir: str, include_headers: bool = False) -> Sequence[Tuple[FileType, str]]:
    file_types = [FileType.C_SOURCE, FileType.CXX_SOURCE]
    if include_headers:
        file_types += (FileType.C_LIKE_HEADER, FileType.CXX_HEADER)
    return find_sources(source_dir, file_types)
