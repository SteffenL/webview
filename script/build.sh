#!/usr/bin/env bash

set -e

script_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
src_dir=$(dirname "${script_dir}")
build_dir=${src_dir}/build
flags=(-Wall -Wextra -pedantic "-I${src_dir}")
cflags=(-std=c99 "${flags[@]}")

if [ "$(uname)" = "Darwin" ]; then
	cxxflags=(-DWEBVIEW_COCOA -std=c++11 "${flags[@]}" -framework WebKit)
else
	cxxflags=(-DWEBVIEW_GTK -std=c++11 "${flags[@]}" $(pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.0))
fi

generator_params=()
if which ninja 2>&1 > /dev/null; then
	generator_params+=(-G Ninja)
fi

if [ -z "${BUILD_EXAMPLES}" ]; then BUILD_EXAMPLES=OFF; fi
if [ -z "${ENABLE_TESTS}" ]; then ENABLE_TESTS=OFF; fi
if [ -z "${ENABLE_GO}" ]; then ENABLE_GO=OFF; fi

if [ "${#@}" = "0" ]; then
	# This is here for backward-compatibility.
	BUILD_EXAMPLES=ON ENABLE_TESTS=ON ENABLE_GO=ON "$0" reformat lint build test || exit 1
	"$0" test || exit 1
	exit 0
fi

for cmd in ${@}; do
	if [ "${cmd}" = "clean" ]; then
		echo "Cleaning..."
		if [ -d "${build_dir}" ]; then
			rm -rf "${build_dir}" || exit 1
		fi
	elif [ "${cmd}" = "reformat" ]; then
		echo "Formatting..."
		clang-format -i \
			"${src_dir}/webview.h" \
			"${src_dir}/examples/c/"*.c \
			"${src_dir}/examples/cc/"*.cc \
			"${src_dir}/test/"*.cc || exit 1
	elif [ "${cmd}" = "lint" ]; then
		echo "Linting..."
		clang-tidy "${src_dir}/examples/cc/basic.cc" -- "${cxxflags[@]}"
		clang-tidy "${src_dir}/examples/cc/bind.cc" -- "${cxxflags[@]}"
		clang-tidy "${src_dir}/test/basic.cc" -- "${cxxflags[@]}"
		clang-tidy "${src_dir}/test/bind.cc" -- "${cxxflags[@]}"
	elif [ "${cmd}" = "configure" ]; then
		echo "Configuring..."
		cmake "${generator_params[@]}" -B "${build_dir}" -S "${src_dir}" \
			"-DBUILD_EXAMPLES=${BUILD_EXAMPLES}" \
			"-DENABLE_TESTS=${ENABLE_TESTS}" \
			"-DENABLE_GO=${ENABLE_GO}" || exit 1
	elif [ "${cmd}" = "build" ]; then
		echo "Building..."
		cmake --build "${build_dir}" || exit 1
	elif [ "${cmd}" = "test" ]; then
		echo "Running tests..."
		ctest --test-dir "${build_dir}" --output-on-failure || exit 1
	else
		echo "Invalid command: $1"
		exit 1
	fi
done
