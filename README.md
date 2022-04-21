# SteffenL's webview

A fork of [webview](https://github.com/webview/webview), a tiny cross-platform webview library for C/C++ to build modern cross-platform GUIs.

This project aims to provide a simple C/C++ library/framework that allows users to create desktop applications based on web technology and platform-provided browser engines such as WebKit on macOS, WebKitGTK on Linux and Edge on Windows >= 10. I am also open to improving things beyond the original scope of the library for the benefit of everyone.

The library supports two-way JavaScript bindings (to call JavaScript from C/C++ and to call C/C++ from JavaScript).

[Why create a fork?](#why-create-a-fork)

## Development

This project uses CMake and Ninja. See instructions on building below.

### Prerequisites

#### Linux (Ubuntu 20.04)

Development packages: `libwebkit2gtk-4.0-dev`
Production packages: `libwebkit2gtk-4.0`

#### Windows

Make sure to run `scripts\windows\fetch_webview2.bat` first to retrieve the Microsoft Edge WebView2 loader library. You will also need the [WebView2 Runtime](https://go.microsoft.com/fwlink/p/?LinkId=2124703) both for development and distribution to client machines. For your convenience you can install the runtime with `MicrosoftEdgeWebview2Setup.exe /silent /install`.

You need Windows 10 SDK >= 1803 (10.0.17134.12) in order to use WebView2 (specify to CMake with `CMAKE_SYSTEM_VERSION`).

### Building

```
cmake -G Ninja -B build -S .
cmake --build build --config Release
ctest --test-dir build/test --output-on-failure --config Release
```

### Build options

The options below can be specified during CMake configuration.

#### `USE_WEBVIEW2_SHARED_LIBRARY`

Set to `ON` to use the Microsoft Edge WebView2 loader as a shared library on Windows; otherwise `OFF` (default: `OFF`).

## Why create a fork?

Here are my main reasons for going my own way with [webview](https://github.com/webview/webview) as of April 20, 2022:

* I wanted to clean up the project structure.
* [Different views on implementing build systems](https://github.com/webview/webview/pull/699) such as Meson and CMake.
* Less optimal coding practices and bugs:
  * [Use reference to const string](https://github.com/webview/webview/pull/712)
  * [Do not display "Hello" when URL is empty](https://github.com/webview/webview/pull/713)
  * [Use reference to const auto](https://github.com/webview/webview/pull/715)
  * [Fix usage of hstring after its destruction on Windows](https://github.com/webview/webview/pull/716)
  * [Fix encoding of data URL in C example](https://github.com/webview/webview/pull/718)
* When using the library as a header-only library then by default C API function definitions caused multiple definition errors when compiling in C++ mode.
* Lack of engagement around issues and pull requests.

All I really wanted to do was move forward with creating my own applications, but when I realized that I had invested more time and effort on submitting pull requests for [webview](https://github.com/webview/webview) than to actually build my application, that was when I decided to branch off.

Major differences between this library and [webview](https://github.com/webview/webview) as of April 20, 2022:

* *This project* uses the CMake build system while the *other project* uses none. Maybe the *other project* [will use Meson in the future](https://github.com/webview/webview/pull/699) if their views on build systems change.
* *This project* aims to improve platform integration and behaviors while the *other project* is more minimalist.
* *This project* mainly focuses on C/C++ while the *other project* also has a lot of room for [Go](https://go.dev/).
