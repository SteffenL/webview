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

project_dir="$(dirname "$(dirname "$(realpath_wrapper "${BASH_SOURCE[0]}")")")" || exit 1
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

echo "-- Project directory: ${project_dir[@]}"
echo "-- C compiler: ${c_compiler}"
echo "-- C compiler flags: ${c_compile_flags[@]}"
echo "-- C linker flags: ${c_link_flags[@]}"
echo "-- C++ compiler: ${cxx_compiler}"
echo "-- C++ compiler flags: ${cxx_compile_flags[@]}"
echo "-- C++ linker flags: ${cxx_link_flags[@]}"

mkdir -p build/examples/c build/examples/cc build/examples/go || true

echo "Building C++ examples"
"${cxx_compiler}" "${project_dir}/examples/basic.cc" "${cxx_compile_flags[@]}" "${cxx_link_flags[@]}" -o "${project_dir}/build/examples/cc/basic" || exit 1
"${cxx_compiler}" "${project_dir}/examples/bind.cc" "${cxx_compile_flags[@]}" "${cxx_link_flags[@]}" -o "${project_dir}/build/examples/cc/bind" || exit 1

echo "Building C examples"
"${cxx_compiler}" -c "${cxx_compile_flags[@]}" "${project_dir}/webview.cc" -o "${project_dir}/build/webview.o" || exit 1
"${c_compiler}" -c "${c_compile_flags[@]}" "${project_dir}/examples/basic.c" -o "${project_dir}/build/examples/c/basic.o" || exit 1
"${c_compiler}" -c "${c_compile_flags[@]}" "${project_dir}/examples/bind.c" -o "${project_dir}/build/examples/c/bind.o" || exit 1
"${cxx_compiler}" "${cxx_compile_flags[@]}" "${project_dir}/build/examples/c/basic.o" "${project_dir}/build/webview.o" "${cxx_link_flags[@]}" -o "${project_dir}/build/examples/c/basic" || exit 1
"${cxx_compiler}" "${cxx_compile_flags[@]}" "${project_dir}/build/examples/c/bind.o" "${project_dir}/build/webview.o" "${cxx_link_flags[@]}" -o "${project_dir}/build/examples/c/bind" || exit 1

echo "Building test app"
"${cxx_compiler}" "${cxx_compile_flags[@]}" "${project_dir}/webview_test.cc" "${cxx_link_flags[@]}" -o "${project_dir}/build/webview_test" || exit 1
