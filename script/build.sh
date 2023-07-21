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
    rm -rd "${build_dir}" || return 1
}

task_build() {
    #rm -rd "${build_dir}" || return 1
    echo build
}

task_test() {
    #rm -rd "${build_dir}" || return 1
    echo test
}

run_task() {
    local name=${1}
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
    cxx_link_flags+=(-mwindows -ladvapi32 -lole32 -lshell32 -lshlwapi -luser32 -lversion)
fi

tasks=(clean build test)

echo "-- C compiler: ${c_compiler}"
echo "-- C compiler flags: ${c_compile_flags[@]}"
echo "-- C linker flags: ${c_link_flags[@]}"
echo "-- C++ compiler: ${cxx_compiler}"
echo "-- C++ compiler flags: ${cxx_compile_flags[@]}"
echo "-- C++ linker flags: ${cxx_link_flags[@]}"

for task in "${tasks[@]}"; do
    run_task "${task}" || exit 1
done
