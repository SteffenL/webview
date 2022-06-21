@echo off
setlocal enabledelayedexpansion

call :main %* || exit /b
goto :eof

:get_remaining_args out_var
    set __result__=
    for /L %%i in (!argi!,1,!argi_end!) do (
        set /A argi+=1
        set "__result__=!__result__! !arg_%%i!"
    )
    if not "!__result__!" == "" set "__result__=!__result__:~1!"
    set "%~1=!__result__!"
    goto :eof

:check_configured
    if not exist "!build_dir!\CMakeCache.txt" (
        echo Please configure the build.
        exit /b 1
    )
    goto :eof

:is_build_examples_enabled out_var
    call :check_configured || exit /b
    set "%~1=false"
    findstr "BUILD_EXAMPLES:BOOL=ON" "!build_dir!\CMakeCache.txt" > nul && set "%~1=true" || exit /b 0
    goto :eof

:check_tests_enabled
    call :check_configured || exit /b
    findstr "ENABLE_TESTS:BOOL=ON" "!build_dir!\CMakeCache.txt" > nul && exit /b
    echo Tests are not enabled.
    exit /b 1

:print_help
    echo This is the build script for the webview library.
    echo.
    echo Usage:
    echo     program COMMAND...
    echo.
    echo Commands:
    echo     build                     Build.
    echo     clean                     Clean build directory.
    echo     configure                 Configure build.
    echo     devenv                    Apply development environment variables.
    echo     exec ARG...               Execute command.
    echo     go:build                  Build with Go.
    echo     go:run FILE               Run Go file.
    echo     go:test                   Run tests with Go.
    echo     help                      Display this help text.
    echo     lint                      Run lint checks (not yet implemented).
    echo     reformat                  Reformat code (not yet implemented).
    echo     test                      Run tests.
    echo.
    echo Configure Options:
    echo    BUILD_EXAMPLES=ON          Build examples.
    echo    ENABLE_TESTS=ON            Enable tests.
    echo    WEBVIEW2_VERSION=VERSION   WebView2 version to use.
    goto :eof

:main arg...
    set script_path=%~0
    set script_dir=%~dp0
    set script_dir=%script_dir:~0,-1%
    set src_dir=!script_dir!\..
    set build_dir=!src_dir!\build
    if defined CI (set test_timeout=60) else (set test_timeout=10)
    set go_ldflags=-ldflags="-H windowsgui"
    set webview2_dir=!build_dir!/_deps/microsoft_web_webview2-src
    set "PATH=!PATH!;!webview2_dir!/build/native/x64;!webview2_dir!/build/native/x86"

    rem Default behavior when no command line arguments have been specified.
    if "%~1" == "" (
        set BUILD_EXAMPLES=ON
        set ENABLE_TESTS=ON
        call "!script_path!" clean reformat lint configure build test go:build go:test
        goto :eof
    )

    set argc=0
    :argc_loop
        if "%~1" == "" goto :argc_loop_end
        set "arg=%1"
        set "arg_!argc!=!arg!"
        set /A argc+=1
        shift
        goto :argc_loop
    :argc_loop_end
    set /A argi_end=argc - 1

    set argi=0
:arg_loop
    if !argi! geq !argc! goto :arg_loop_end
    set arg=!arg_%argi%!
    if "!arg!" == "build" (
        call :check_configured || exit /b
        echo Building...
        cmake --build "!build_dir!" || exit /b
    ) else if "!arg!" == "clean" (
        echo Cleaning...
        if exist "!build_dir!" (
            rmdir /q /s "!build_dir!" || exit /b
        )
    ) else if "!arg!" == "configure" (
        echo Configuring...
        set generator_params=
        where ninja 2>&1 > nul && set generator_params=-G Ninja
        cmd /c exit 0
        set config_params=
        if not "!BUILD_EXAMPLES!" == "" set config_params=!config_params! "-DBUILD_EXAMPLES=!BUILD_EXAMPLES!"
        if not "!ENABLE_TESTS!" == "" set config_params=!config_params! "-DENABLE_TESTS=!ENABLE_TESTS!"
        if not "!WEBVIEW2_VERSION!" == "" set config_params=!config_params! "-DWEBVIEW2_VERSION=!WEBVIEW2_VERSION!"
        cmake !generator_params! -B "!build_dir!" -S "!src_dir!" !config_params! || exit /b
    ) else if "!arg!" == "devenv" (
        preserve_vars_on_exit=PATH
    ) else if "!arg!" == "exec" (
        set /A argi+=1
        call :get_remaining_args remaining_args || exit /b
        echo Executing: !remaining_args!
        call !remaining_args! || exit /b
    ) else if "!arg!" == "go:build" (
        call :is_build_examples_enabled build_examples_enabled
        echo Go: Building...
        rem go build !go_ldflags! || exit /b
        if "!build_examples_enabled!" == "true" (
            if not exist "!build_dir!\examples\go" mkdir "!build_dir!\examples\go" || exit /b
            for /R "%src_dir%\examples\go" %%f in (*.go) do (
                go build !go_ldflags! -o "!build_dir!\examples\go\%%~nf.exe" "%%~f" || exit /b
            )
        )
    ) else if "!arg!" == "go:run" (
        call :check_configured || exit /b
        set /A argi+=1
        call :get_remaining_args remaining_args || exit /b
        echo Go: Running: !remaining_args!
        go run !go_ldflags! !remaining_args! || exit /b
    ) else if "!arg!" == "go:test" (
        call :check_tests_enabled || exit /b
        call :check_configured || exit /b
        echo Go: Running tests...
        go test || exit /b
    ) else if "!arg!" == "help" (
        call :print_help || exit /b
    ) else if "!arg!" == "lint" (
        rem Not yet implemented
    ) else if "!arg!" == "reformat" (
        rem Not yet implemented
    ) else if "!arg!" == "test" (
        call :check_tests_enabled || exit /b
        call :check_configured || exit /b
        echo Running tests...
        ctest --test-dir "!build_dir!" --output-on-failure --timeout "!test_timeout!" || exit /b
    ) else (
        echo Invalid command: !arg!
        echo Try the "help" command.
        exit /b 1
    )
    set /A argi+=1
    goto :arg_loop
:arg_loop_end
    if not "!preserve_vars_on_exit!" == "" (
        set cmd=echo Applying development environment variables...
        for %%a in (!preserve_vars_on_exit!) (
            
        )
    )
    goto :eof
