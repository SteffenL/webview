#!/bin/sh

set -e

DIR="$(cd "$(dirname "$0")/../" && pwd)"
INCLUDE_DIR=$DIR/include
SOURCE_DIR=$DIR
BUILD_DIR=$DIR/build
BUILD_INT_DIR=$DIR/build/intermediate
BUILD_LIB_DIR=$DIR/build/lib
BUILD_BIN_DIR=$BUILD_DIR/bin
TEST_SOURCE_DIR=$DIR/test
TEST_BIN_DIR=$BUILD_BIN_DIR/test
EXAMPLES_SOURCE_DIR=$DIR/examples
EXAMPLES_INT_DIR=$BUILD_INT_DIR/examples
EXAMPLES_BIN_DIR=$BUILD_BIN_DIR/examples

if [ "$(uname)" = "Darwin" ]; then
	FLAGS="-DWEBVIEW_COCOA -std=c++11 -Wall -Wextra -pedantic -framework WebKit -I$INCLUDE_DIR"
else
	FLAGS="-DWEBVIEW_GTK -std=c++11 -Wall -Wextra -pedantic $(pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.0) -I$INCLUDE_DIR"
fi

if [ -z "${SKIP_CHECK}" ]; then
	if command -v clang-format >/dev/null 2>&1 ; then
		echo "Formatting..."
		find . -type f -regextype posix-extended -iregex '.*\.(h|cc?)$' -exec clang-format -i {} +
	else
		echo "SKIP: Formatting (clang-format not installed)"
	fi
fi

if [ -z "${SKIP_LINT}" ]; then
	if command -v clang-tidy >/dev/null 2>&1 ; then
		echo "Linting..."
		find . -type f -regextype posix-extended -iregex '.*\.cc$' -exec clang-tidy {} -- $FLAGS \;
	else
		echo "SKIP: Linting (clang-tidy not installed)"
	fi
fi

mkdir --parents "$TEST_BIN_DIR" "$EXAMPLES_INT_DIR" "$EXAMPLES_BIN_DIR"

echo "Building C++ example"
c++ "$EXAMPLES_SOURCE_DIR/main.cc" $FLAGS -o "$EXAMPLES_BIN_DIR/cpp_example"

echo "Building C example"
c++ -c "$SOURCE_DIR/webview.cc" $FLAGS -o "$BUILD_INT_DIR/webview.o"
cc -c "$EXAMPLES_SOURCE_DIR/main.c" -o "$EXAMPLES_INT_DIR/c_example.o" "-I$INCLUDE_DIR"
c++ "$EXAMPLES_INT_DIR/c_example.o" "$BUILD_INT_DIR/webview.o" $FLAGS -o "$EXAMPLES_BIN_DIR/c_example"

echo "Building test app"
c++ "$TEST_SOURCE_DIR/webview_test.cc" $FLAGS -o "$TEST_BIN_DIR/webview_test"

if [ -z "${SKIP_TEST}" ]; then
	echo "Running tests"
	find "$TEST_BIN_DIR" -type f -exec {} \;

	if command -v go >/dev/null 2>&1 ; then
		echo "Running Go tests"
		CGO_ENABLED=1 go test -v
	else
		echo "SKIP: Go tests"
	fi
fi
