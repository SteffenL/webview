#!/bin/sh

set -e

DIR="$(cd "$(dirname "$0")/../" && pwd)"
INCLUDE_DIR=$DIR/include
SOURCE_DIR=$DIR/src
BUILD_DIR=$DIR/build
BUILD_INT_DIR=$DIR/build/intermediate
BUILD_LIB_DIR=$DIR/build/lib
BUILD_BIN_DIR=$BUILD_DIR/bin
STATIC_LIB_DIR=$BUILD_LIB_DIR/static
SHARED_LIB_DIR=$BUILD_LIB_DIR/shared
EXAMPLES_SOURCE_DIR=$SOURCE_DIR/examples
EXAMPLES_INT_DIR=$BUILD_INT_DIR/examples
EXAMPLES_BIN_DIR=$BUILD_BIN_DIR/examples
TEST_SOURCE_DIR=$DIR/test

if [ "$(uname)" = "Darwin" ]; then
	FLAGS="-DWEBVIEW_COCOA -std=c++11 -Wall -Wextra -pedantic -framework WebKit -I$DIR"
else
	FLAGS="-DWEBVIEW_GTK -std=c++11 -Wall -Wextra -pedantic $(pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.0) -I$DIR"
fi

STATIC_LIBRARY_FLAGS="-L$STATIC_LIB_DIR -lwebview"

if command -v clang-format >/dev/null 2>&1 ; then
	echo "Formatting..."
	find . -type f -regextype posix-extended -iregex '.*\.(h|cc?)$' -exec clang-format -i {} +
else
	echo "SKIP: Formatting (clang-format not installed)"
fi

if command -v clang-tidy >/dev/null 2>&1 ; then
	echo "Linting..."
	find . -type f -regextype posix-extended -iregex '.*\.cc$' -exec clang-tidy {} -- $FLAGS \;
else
	echo "SKIP: Linting (clang-tidy not installed)"
fi

mkdir --parents "$STATIC_LIB_DIR" "$SHARED_LIB_DIR" "$EXAMPLES_INT_DIR" "$EXAMPLES_BIN_DIR"

echo "Building library"
c++ -c "$SOURCE_DIR/webview.cc" $FLAGS -o "$BUILD_INT_DIR/webview.o"
ar -r "$STATIC_LIB_DIR/libwebview.a" "$BUILD_INT_DIR/webview.o"
c++ -shared -fvisibility=hidden -fPIC -DWEBVIEW_EXPORTING "$SOURCE_DIR/webview.cc" $FLAGS -o "$SHARED_LIB_DIR/libwebview.so"

echo "Building C++ example"
c++ "$EXAMPLES_SOURCE_DIR/main.cc" $STATIC_LIBRARY_FLAGS $FLAGS -o "$EXAMPLES_BIN_DIR/webview_cc_example"

echo "Building C example"
cc -c "$EXAMPLES_SOURCE_DIR/main.c" -o "$EXAMPLES_INT_DIR/main.o" "-I$INCLUDE_DIR"
c++ "$EXAMPLES_INT_DIR/main.o" $STATIC_LIBRARY_FLAGS $FLAGS -o "$EXAMPLES_BIN_DIR/webview_c_example"

echo "Building test app"
c++ "$TEST_SOURCE_DIR/webview_test.cc" $STATIC_LIBRARY_FLAGS $FLAGS -o "$BUILD_BIN_DIR/webview_test"

echo "Running tests"
"$BUILD_BIN_DIR/webview_test"

exit 0
if command -v go >/dev/null 2>&1 ; then
	echo "Running Go tests"
	CGO_ENABLED=1 go install ./bindings/go/package
	CGO_ENABLED=1 go test -v ./bindings/go/test
else
	echo "SKIP: Go tests"
fi
