#!/bin/bash

if [[ "${OSTYPE}" == "msys" || "${OSTYPE}" == "cygwin" ]]; then
    os=windows
elif [[ "$(uname)" == "Darwin" ]]; then
    os=macos
else
    os=linux
fi

realpath_wrapper() {
    if [[ "${os}" == "macos" ]]; then
        readlink -f "${1}" || return 1
    else
        realpath "${1}" || return 1
    fi
}

task_clean() {
    if [[ -d "${build_dir}" ]]; then
        rm -rd "${build_dir}" || return 1
    fi
}

task_format() {
    if command -v clang-format >/dev/null 2>&1 ; then
        echo "Formatting..."
        clang-format -i \
                "${project_dir}/webview.h" \
                "${project_dir}/webview_test.cc" \
                "${project_dir}/examples/"*.c \
                "${project_dir}/examples/"*.cc || return 1
    else
        echo "SKIP: Formatting (clang-format not installed)"
    fi
}

task_check() {
    if command -v clang-tidy >/dev/null 2>&1 ; then
        echo "Linting..."
        clang-tidy "${project_dir}/examples/basic.cc" -- "${cxx_compile_flags[@]}" "${cxx_link_flags[@]}" || return 1
        clang-tidy "${project_dir}/examples/bind.cc" -- "${cxx_compile_flags[@]}" "${cxx_link_flags[@]}" || return 1
        clang-tidy "${project_dir}/webview_test.cc" -- "${cxx_compile_flags[@]}" "${cxx_link_flags[@]}" || return 1
    else
        echo "SKIP: Linting (clang-tidy not installed)"
    fi
}

task_build() {
    mkdir -p build/examples/c build/examples/cc build/examples/go || true

    echo "Building C++ examples"
    "${cxx_compiler}" "${project_dir}/examples/basic.cc" "${cxx_compile_flags[@]}" "${cxx_link_flags[@]}" -o "${build_dir}/examples/cc/basic${exe_suffix}" || exit 1
    "${cxx_compiler}" "${project_dir}/examples/bind.cc" "${cxx_compile_flags[@]}" "${cxx_link_flags[@]}" -o "${build_dir}/examples/cc/bind${exe_suffix}" || exit 1

    echo "Building C examples"
    "${cxx_compiler}" -c "${cxx_compile_flags[@]}" "${project_dir}/webview.cc" -o "${build_dir}/webview.o" || exit 1
    "${c_compiler}" -c "${c_compile_flags[@]}" "${project_dir}/examples/basic.c" -o "${build_dir}/examples/c/basic.o" || exit 1
    "${c_compiler}" -c "${c_compile_flags[@]}" "${project_dir}/examples/bind.c" -o "${build_dir}/examples/c/bind.o" || exit 1
    "${cxx_compiler}" "${cxx_compile_flags[@]}" "${build_dir}/examples/c/basic.o" "${build_dir}/webview.o" "${cxx_link_flags[@]}" -o "${build_dir}/examples/c/basic${exe_suffix}" || exit 1
    "${cxx_compiler}" "${cxx_compile_flags[@]}" "${build_dir}/examples/c/bind.o" "${build_dir}/webview.o" "${cxx_link_flags[@]}" -o "${build_dir}/examples/c/bind${exe_suffix}" || exit 1

    echo "Building test app"
    "${cxx_compiler}" "${cxx_compile_flags[@]}" "${project_dir}/webview_test.cc" "${cxx_link_flags[@]}" -o "${build_dir}/webview_test${exe_suffix}" || exit 1
}

task_test() {
    echo "Running tests..."
    "${build_dir}/webview_test${exe_suffix}" || return 1
}

task_go_build() {
    if command -v go >/dev/null 2>&1 ; then
        echo "Building Go examples..."
        (cd "${project_dir}" && (
            go build -o "build/examples/go/basic${exe_suffix}" examples/basic.go || exit 1
            go build -o "build/examples/go/bind${exe_suffix}" examples/bind.go || exit 1
        )) || return 1
    else
        echo "SKIP: Go build (go not installed)"
    fi
}

task_go_test() {
    if command -v go >/dev/null 2>&1 ; then
        echo "Running Go tests..."
        CGO_ENABLED=1 go test
    else
        echo "SKIP: Go tests (go not installed)"
    fi
}

run_task() {
    local name=${1/:/_}
    shift
    eval "task_${name}" "${@}" || return 1
}

# Default C standard
c_std=c99
# Default C++ standard
cxx_std=c++11
# Default C compiler
c_compiler=cc
# Default C++ compiler
cxx_compiler=c++

# C compiler override
if [[ ! -z "${CC}" ]]; then
    c_compiler=${CC}
fi

# C++ compiler override
if [[ ! -z "${CXX}" ]]; then
    cxx_compiler=${CXX}
fi

project_dir=$(dirname "$(dirname "$(realpath_wrapper "${BASH_SOURCE[0]}")")") || exit 1
build_dir=${project_dir}/build
warning_flags=(-Wall -Wextra -pedantic)
common_compile_flags=("${warning_flags[@]}" "-I${project_dir}")
common_link_flags=("${warning_flags[@]}")
c_compile_flags=("${common_compile_flags[@]}")
c_link_flags=("${common_link_flags[@]}")
cxx_compile_flags=("${common_compile_flags[@]}")
cxx_link_flags=("${common_link_flags[@]}")
exe_suffix=

c_compile_flags+=("-std=${c_std}")
cxx_compile_flags+=("-std=${cxx_std}")

if [[ "${os}" == "linux" ]]; then
    pkgconfig_libs=(gtk+-3.0 webkit2gtk-4.0)
    cxx_compile_flags+=($(pkg-config --cflags "${pkgconfig_libs[@]}")) || exit 1
    cxx_link_flags+=($(pkg-config --libs "${pkgconfig_libs[@]}")) || exit 1
elif [[ "${os}" == "macos" ]]; then
    cxx_link_flags+=(-framework WebKit)
    macos_target_version=10.9
    c_compile_flags+=("-mmacosx-version-min=${macos_target_version}")
    cxx_compile_flags+=("-mmacosx-version-min=${macos_target_version}")
elif [[ "${os}" == "windows" ]]; then
    exe_suffix=.exe
    cxx_link_flags+=(-mwindows -ladvapi32 -lole32 -lshell32 -lshlwapi -luser32 -lversion)
fi

# Default tasks
tasks=(clean format check build test go:build go:test)

# Task override from command line
if [[ ${#@} -gt 0 ]]; then
    tasks=("${@}")
fi

echo "-- C compiler: ${c_compiler}"
echo "-- C compiler flags: ${c_compile_flags[@]}"
echo "-- C linker flags: ${c_link_flags[@]}"
echo "-- C++ compiler: ${cxx_compiler}"
echo "-- C++ compiler flags: ${cxx_compile_flags[@]}"
echo "-- C++ linker flags: ${cxx_link_flags[@]}"

for task in "${tasks[@]}"; do
    run_task "${task}" || exit 1
done
