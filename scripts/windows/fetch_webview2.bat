@echo off

set "external_dir=%~dp0..\..\external"
set "nuget_dir=%external_dir%\nuget"
set webview2_name=Microsoft.Web.Webview2
set webview2_version=1.0.1185.39
set webview2_arch=x64
set "webview2_target_dir=%external_dir%\%webview2_name%"

if not exist "%external_dir%" mkdir "%external_dir%" || call :fail "Unable to create external directory"

set "webview2_nuget_dir=%nuget_dir%\%webview2_name%.%webview2_version%"
if not exist "%webview2_nuget_dir%" (
    echo Fetching %webview2_name% %webview2_version%...
    nuget install "%webview2_name%" -Version "%webview2_version%" -OutputDirectory "%nuget_dir%" || call :fail "Unable to fetch nuget package"
)

call :delete_if_exists "%webview2_target_dir%" || call :fail "Unable to delete existing target directory"

mkdir "%webview2_target_dir%" || call :fail "Unable to create target directory"
mkdir "%webview2_target_dir%\bin" "%webview2_target_dir%\lib" "%webview2_target_dir%\include" || call :fail "Unable to create target subdirectories"
copy /Y "%webview2_nuget_dir%\build\native\include" "%webview2_target_dir%\include" > nul || call :fail "Unable to copy include files"
copy /Y "%webview2_nuget_dir%\build\native\%webview2_arch%\*.dll" "%webview2_target_dir%\bin" > nul || call :fail "Unable to copy bin files"
copy /Y "%webview2_nuget_dir%\build\native\%webview2_arch%\*.lib" "%webview2_target_dir%\lib" > nul || call :fail "Unable to copy lib files"
rename "%webview2_target_dir%\lib\WebView2Loader.dll.lib" "WebView2Loader.lib" > nul || call :fail "Unable to rename lib file"

goto :eof

:delete_if_exists
    if exist "%~1" (
        rmdir /S /Q "%~1" || exit /B 1
    )
    exit /B 0

:fail
    echo An error occurred: %~1
    call :exit_with_failure 2> nul

:exit_with_failure
    () :: Abuse syntax error to exit script
