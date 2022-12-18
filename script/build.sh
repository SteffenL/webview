#!/bin/sh

set -e

DIR="$(cd "$(dirname "$0")/../" && pwd)"
BUILD_DIR=$DIR/build
FLAGS="-Wall -Wextra -pedantic -I$DIR"
CFLAGS="-std=c99 $FLAGS"

if [ "$(uname)" = "Darwin" ]; then
	CXXFLAGS="-DWEBVIEW_COCOA -std=c++11 $FLAGS -framework WebKit"
else
	CXXFLAGS="-DWEBVIEW_GTK -std=c++11 $FLAGS $(pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.0)"
fi

if command -v clang-format >/dev/null 2>&1 ; then
	echo "Formatting..."
	clang-format -i \
		"$DIR/webview.h" \
		"$DIR/webview_test.cc" \
		"$DIR/examples/"*.c \
		"$DIR/examples/"*.cc
else
	echo "SKIP: Formatting (clang-format not installed)"
fi

if command -v clang-tidy >/dev/null 2>&1 ; then
	echo "Linting..."
	clang-tidy "$DIR/examples/basic.cc" -- $CXXFLAGS
	clang-tidy "$DIR/examples/bind.cc" -- $CXXFLAGS
	clang-tidy "$DIR/webview_test.cc" -- $CXXFLAGS
else
	echo "SKIP: Linting (clang-tidy not installed)"
fi

mkdir -p "$BUILD_DIR/examples/c" "$BUILD_DIR/examples/cc" "$BUILD_DIR/examples/go" || true

echo "Building C++ examples"
c++ "$DIR/examples/basic.cc" $CXXFLAGS -o "$BUILD_DIR/examples/cc/basic"
c++ "$DIR/examples/bind.cc" $CXXFLAGS -o "$BUILD_DIR/examples/cc/bind"

echo "Building C examples"
c++ -c $CXXFLAGS "$DIR/webview.cc" -o "$BUILD_DIR/webview.o"
cc -c "$DIR/examples/basic.c" $CFLAGS -o "$BUILD_DIR/examples/c/basic.o"
cc -c "$DIR/examples/bind.c" $CFLAGS -o "$BUILD_DIR/examples/c/bind.o"
c++ "$BUILD_DIR/examples/c/basic.o" "$BUILD_DIR/webview.o" $CXXFLAGS -o "$BUILD_DIR/examples/c/basic"
c++ "$BUILD_DIR/examples/c/bind.o" "$BUILD_DIR/webview.o" $CXXFLAGS -o "$BUILD_DIR/examples/c/bind"

# Go needs go.mod to be in the working directory.
cd "$DIR"

echo "Building Go examples"
go build -o "$BUILD_DIR/examples/go/basic" examples/basic.go
go build -o "$BUILD_DIR/examples/go/bind" examples/bind.go

echo "Building test app"
c++ "$DIR/webview_test.cc" $CXXFLAGS -o "$BUILD_DIR/webview_test"

echo "Running tests"
"$BUILD_DIR/webview_test"

if command -v go >/dev/null 2>&1 ; then
	echo "Running Go tests"
	CGO_ENABLED=1 go test
else
	echo "SKIP: Go tests"
fi
