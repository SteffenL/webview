from internal.common import Arch

from typing import Tuple

GO_ARCH_MAP = {
    Arch.ARM64: "arm64",
    Arch.ARM32: "arm",
    Arch.X64: "amd64",
    Arch.X86: "x86"
}

def to_go_architecture(architecture: Arch) -> str:
    return GO_ARCH_MAP[architecture]

def parse_go_version_string(s: str) -> Tuple[int, int, int]:
    words = s.strip().split(" ")
    if len(words) < 4 or words[0] != "go" or words[1] != "version":
        raise Exception("Invalid go version string")
    version = words[2].removeprefix("go").split(".")
    version = tuple(int(element) for element in version)
    return version

def go_version_supports_quoted_params(version: Tuple[int, int, int]) -> bool:
    return version >= (1, 18, 0)
