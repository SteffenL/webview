#!/bin/sh

set -e

DIR="$(cd "$(dirname "$0")/../" && pwd)"
INCLUDE_DIR=$DIR
SOURCE_DIR=$DIR
BUILD_DIR=$DIR/build
BUILD_INT_DIR=$BUILD_DIR/intermediate
BUILD_BIN_DIR=$BUILD_DIR/bin
STATIC_BUILD_INT_DIR=$BUILD_DIR/intermediate/static
SHARED_BUILD_INT_DIR=$BUILD_DIR/intermediate/shared
BUILD_LIB_DIR=$BUILD_DIR/lib
STATIC_BUILD_LIB_DIR=$BUILD_LIB_DIR/static
SHARED_BUILD_LIB_DIR=$BUILD_LIB_DIR/shared
TEST_SOURCE_DIR=$DIR/test
TEST_INT_DIR=$BUILD_INT_DIR/test
TEST_BIN_DIR=$BUILD_BIN_DIR/test
EXAMPLES_SOURCE_DIR=$DIR/examples
EXAMPLES_INT_DIR=$BUILD_INT_DIR/examples
EXAMPLES_BIN_DIR=$BUILD_BIN_DIR/examples

CFLAGS="-I$INCLUDE_DIR"
CXXFLAGS="-I$INCLUDE_DIR"
LDFLAGS=""

if [ "$(uname)" = "Darwin" ]; then
	LDFLAGS="-framework WebKit"
	CXXFLAGS="$CXXFLAGS -DWEBVIEW_COCOA -std=c++11 -Wall -Wextra -pedantic"
	SHARED_LIB_FLAG="-dynamiclib"
	SHARED_LIB_NAME_EXTENSION=".dylib"
else
	LDFLAGS="$(pkg-config --libs gtk+-3.0 webkit2gtk-4.0)"
	CXXFLAGS="$CXXFLAGS -DWEBVIEW_GTK -std=c++11 -Wall -Wextra -pedantic $(pkg-config --cflags gtk+-3.0 webkit2gtk-4.0)"
	SHARED_LIB_FLAG="-shared"
	SHARED_LIB_NAME_EXTENSION=".so"
fi

if command -v clang-format >/dev/null 2>&1 ; then
	echo "Formatting..."
	find "$DIR" -type f -regextype posix-extended -iregex '.+\.(h|cc?)$' -exec clang-format --verbose -i {} +
else
	echo "SKIP: Formatting (clang-format not installed)"
fi

if command -v clang-tidy >/dev/null 2>&1 ; then
	echo "Linting..."
	# Run checks on all source files and check those that require special compile-time flags separately
	find "$DIR" -type f -regextype posix-extended -iregex '.+\.c$' -exec clang-tidy {} -- $CFLAGS \;
	find "$DIR" -type f -regextype posix-extended -iregex '.+\.cc$' -a \
		\! -name 'webview_shared_library_test.cc' \
		-exec clang-tidy {} -- $CXXFLAGS \;
	clang-tidy "$TEST_SOURCE_DIR/webview_shared_library_test.cc" -- $CXXFLAGS -DWEBVIEW_SHARED
else
	echo "SKIP: Linting (clang-tidy not installed)"
fi

mkdir -p "$TEST_INT_DIR" "$TEST_BIN_DIR" \
	"$STATIC_BUILD_INT_DIR" "$STATIC_BUILD_LIB_DIR" \
	"$SHARED_BUILD_INT_DIR" "$SHARED_BUILD_LIB_DIR" \
	"$EXAMPLES_INT_DIR" "$EXAMPLES_BIN_DIR"

echo "Building static library"
c++ -c "$SOURCE_DIR/webview.cc" -DWEBVIEW_BUILDING -DWEBVIEW_STATIC $CXXFLAGS -o "$STATIC_BUILD_INT_DIR/webview.o"
ar rcs "$STATIC_BUILD_LIB_DIR/libwebview.a" "$STATIC_BUILD_INT_DIR/webview.o"

echo "Building shared library"
c++ -c "$SOURCE_DIR/webview.cc" -DWEBVIEW_BUILDING -DWEBVIEW_SHARED -fPIC -fvisibility=hidden -fvisibility-inlines-hidden $CXXFLAGS -o "$SHARED_BUILD_INT_DIR/webview.o"
c++ $SHARED_LIB_FLAG "$SHARED_BUILD_INT_DIR/webview.o" $LDFLAGS -o "$SHARED_BUILD_LIB_DIR/libwebview${SHARED_LIB_NAME_EXTENSION}"

echo "Building C++ example using header-only library"
c++ "$EXAMPLES_SOURCE_DIR/main.cc" $CXXFLAGS $LDFLAGS -o "$EXAMPLES_BIN_DIR/cpp_example_header"

echo "Building C++ example using static library"
c++ "$EXAMPLES_SOURCE_DIR/main.cc" "-L$STATIC_BUILD_LIB_DIR" -lwebview -DWEBVIEW_STATIC $CXXFLAGS $LDFLAGS -o "$EXAMPLES_BIN_DIR/cpp_example_static"

echo "Building C example using shared library"
cc -c "$EXAMPLES_SOURCE_DIR/main.c" -DWEBVIEW_SHARED $CFLAGS -o "$EXAMPLES_INT_DIR/c_example.o"
c++ "$EXAMPLES_INT_DIR/c_example.o" "-L$SHARED_BUILD_LIB_DIR" -lwebview $LDFLAGS -o "$EXAMPLES_BIN_DIR/c_example"

echo "Building test app"
c++ "$TEST_SOURCE_DIR/webview_test.cc" $CXXFLAGS $LDFLAGS -o "$TEST_BIN_DIR/webview_test_header"
c++ "$TEST_SOURCE_DIR/webview_test.cc" "-L$STATIC_BUILD_LIB_DIR" -lwebview -DWEBVIEW_STATIC $CXXFLAGS $LDFLAGS -o "$TEST_BIN_DIR/webview_test_static"

echo "Building shared library test"
c++ "$TEST_SOURCE_DIR/webview_shared_library_test.cc" "-L$SHARED_BUILD_LIB_DIR" -lwebview -DWEBVIEW_SHARED $CXXFLAGS $LDFLAGS -o "$TEST_BIN_DIR/webview_shared_library_test"

echo "Building library type test"
c++ "$TEST_SOURCE_DIR/webview_library_type_test.cc" "-L$SHARED_BUILD_LIB_DIR" -lwebview -DWEBVIEW_SHARED $CXXFLAGS $LDFLAGS -o "$TEST_BIN_DIR/webview_library_type_test_shared"
c++ "$TEST_SOURCE_DIR/webview_library_type_test.cc" "-L$STATIC_BUILD_LIB_DIR" -lwebview -DWEBVIEW_STATIC $CXXFLAGS $LDFLAGS -o "$TEST_BIN_DIR/webview_library_type_test_static"
c++ "$TEST_SOURCE_DIR/webview_library_type_test.cc" $CXXFLAGS $LDFLAGS -o "$TEST_BIN_DIR/webview_library_type_test_header"

# Run all tests except those that will be handled separately
find "$TEST_BIN_DIR" -type f -not -name "webview_shared_library_test" -not -name "webview_library_type_test_*" | while read f; do
	echo "Running test app: $(basename "$f")"
	"$f"
done

echo -n "Should not be able to run shared library test without shared library: "
if ! "$TEST_BIN_DIR/webview_shared_library_test" > /dev/null 2>&1; then
	echo "OK"
else
	echo "FAILED"; exit 1
fi

echo -n "Should be able to run shared library test using LD_LIBRARY_PATH: "
if LD_LIBRARY_PATH="$SHARED_BUILD_LIB_DIR" "$TEST_BIN_DIR/webview_shared_library_test" > /dev/null 2>&1; then
	echo "OK"
else
	echo "FAILED"; exit 1
fi

echo -n "Checking expected compiler options for shared library: "
OUTPUT=$(LD_LIBRARY_PATH="$SHARED_BUILD_LIB_DIR" "$TEST_BIN_DIR/webview_library_type_test_shared")
if echo "$OUTPUT" | grep -q "Type: shared" && echo "$OUTPUT" | grep -q "Implementation included: no" && echo "$OUTPUT" | grep -q "Implementation opt-out: no"; then
	echo "OK"
else
	echo "FAILED"; exit 1
fi

echo -n "Checking expected compiler options for static library: "
OUTPUT=$("$TEST_BIN_DIR/webview_library_type_test_static")
if echo "$OUTPUT" | grep -q "Type: static" && echo "$OUTPUT" | grep -q "Implementation included: yes" && echo "$OUTPUT" | grep -q "Implementation opt-out: no"; then
	echo "OK"
else
	echo "FAILED"; exit 1
fi

echo -n "Checking expected compiler options for header-only library: "
OUTPUT=$("$TEST_BIN_DIR/webview_library_type_test_header")
if echo "$OUTPUT" | grep -q "Type: header-only" && echo "$OUTPUT" | grep -q "Implementation included: yes" && echo "$OUTPUT" | grep -q "Implementation opt-out: no"; then
	echo "OK"
else
	echo "FAILED"; exit 1
fi

if command -v go >/dev/null 2>&1 ; then
	echo "Running Go tests"
	CGO_ENABLED=1 go test
else
	echo "SKIP: Go tests"
fi
