#!/usr/bin/env bash

set -e

is_true_string() {
	if [[ "$1" =~ 1|true|yes ]]; then
		return 0
	fi
	return 1
}

DIR="$(cd "$(dirname "$0")/../" && pwd)"
EXAMPLES_DIR=$DIR/examples
BUILD_DIR=$DIR/build
BUILD_BIN_DIR=$BUILD_DIR/bin
BUILD_LIB_DIR=$BUILD_DIR/lib
BUILD_OBJ_DIR=$BUILD_DIR/obj

# Default values for options
if [[ -z "$ENABLE_CLANG_FORMAT" ]]; then
	if [[ ! -z "$CI" ]]; then
		ENABLE_CLANG_FORMAT=true
	else
		ENABLE_CLANG_FORMAT=false
	fi
fi
if [[ -z "$ENABLE_CLANG_TIDY" ]]; then
	if [[ ! -z "$CI" ]]; then
		ENABLE_CLANG_TIDY=true
	else
		ENABLE_CLANG_TIDY=false
	fi
fi
if [[ -z "$BUILD_EXAMPLES" ]]; then
	if [[ ! -z "$CI" ]]; then
		BUILD_EXAMPLES=true
	else
		BUILD_EXAMPLES=false
	fi
fi
if [[ -z "$ENABLE_TESTS" ]]; then
	if [[ ! -z "$CI" ]]; then
		ENABLE_TESTS=true
	else
		ENABLE_TESTS=false
	fi
fi
if [[ -z "$BUILD_TESTS" ]]; then
	BUILD_TESTS=true
fi
if [[ -z "$RUN_TESTS" ]]; then
	RUN_TESTS=true
fi
if [[ -z "$RUN_GO_TESTS" ]]; then
	RUN_GO_TESTS=true
fi

mkdir -p "$BUILD_BIN_DIR" || true
mkdir -p "$BUILD_LIB_DIR" || true
mkdir -p "$BUILD_OBJ_DIR" || true

if [[ "$(uname)" = "Darwin" ]]; then
	FLAGS="-DWEBVIEW_COCOA -std=c++11 -Wall -Wextra -pedantic -framework WebKit"
else
	FLAGS="-DWEBVIEW_GTK -std=c++11 -Wall -Wextra -pedantic $(pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.0)"
fi

# Enable clang-format by default
if is_true_string "$ENABLE_CLANG_FORMAT"; then
	if command -v clang-format >/dev/null 2>&1 ; then
		echo "Formatting..."
		clang-format -i \
			"$DIR/webview.h" \
			"$DIR/webview_test.cc" \
			"$EXAMPLES_DIR/main.cc"
	else
		echo "SKIP: Formatting (clang-format not installed)"
	fi
fi

# Enable clang-tidy by default
if is_true_string "$ENABLE_CLANG_TIDY"; then
	if command -v clang-tidy >/dev/null 2>&1 ; then
		echo "Linting..."
		clang-tidy "$EXAMPLES_DIR/main.cc" -- "-I$DIR" $FLAGS
		clang-tidy "$DIR/webview_test.cc" -- "-I$DIR" $FLAGS
	else
		echo "SKIP: Linting (clang-tidy not installed)"
	fi
fi

# Build examples by default
if is_true_string "$BUILD_EXAMPLES"; then
	echo "Building C++ example"
	c++ "-I$DIR" "$EXAMPLES_DIR/main.cc" $FLAGS -o "$BUILD_BIN_DIR/webview_example_cpp"

	echo "Building C example"
	c++ -c $FLAGS "$DIR/webview.cc" -o "$BUILD_OBJ_DIR/webview.o"
	cc "-I$DIR" -c "$EXAMPLES_DIR/main.c" -o "$BUILD_OBJ_DIR/webview_example_c.o"
	c++ "$BUILD_OBJ_DIR/webview_example_c.o" "$BUILD_OBJ_DIR/webview.o" $FLAGS -o "$BUILD_BIN_DIR/webview_example_cpp"
fi

# Enable tests by default
if is_true_string "$ENABLE_TESTS"; then
	# Build tests by default
	if is_true_string "$BUILD_TESTS"; then
		echo "Building test app"
		c++ "$DIR/webview_test.cc" $FLAGS -o "$BUILD_BIN_DIR/webview_test"
	fi

	# Run tests by default
	if is_true_string "$RUN_TESTS"; then
		echo "Running tests"
		"$BUILD_BIN_DIR/webview_test"
	fi

	# Run Go tests by default
	if is_true_string "$RUN_GO_TESTS"; then
		if command -v go >/dev/null 2>&1 ; then
			echo "Running Go tests"
			CGO_ENABLED=1 go test
		else
			echo "SKIP: Go tests"
		fi
	fi
fi
