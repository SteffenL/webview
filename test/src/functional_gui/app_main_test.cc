// =================================================================
// TEST: ensure that webview_app_main works.
// =================================================================

#include <winuser.h>
#define WEBVIEW_DEFINE_GUI_MAIN
#include "webview/test.h"
#include "webview/webview.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include <windows.h>
#include <debugapi.h>

int webview_app_main(int argc, char *argv[]) {
  REQUIRE(argc > 0);
  REQUIRE(argv);
  auto is_child = argc > 1;
  int exit_code = 0;
  if (is_child) {
    MessageBoxA(0, "asd", "asd");
    std::vector<std::string> expected_args{"foo", "バー", "æøå"};
    REQUIRE(argc == expected_args.size() + 1);
    for (int i = 0; i < argc; ++i) {
      REQUIRE(argv[i]);
      REQUIRE(argv[i] == expected_args[i]);
    }
  } else {
    REQUIRE(argc == 1);
    REQUIRE(argv[0]);
  #ifdef _WIN32
    std::wstring exe_path(MAX_PATH, '\0');
    auto exe_path_length = GetModuleFileNameW(nullptr, exe_path.data(), static_cast<DWORD>(exe_path.size()));
    REQUIRE(exe_path_length > 0);
    exe_path.resize(exe_path_length);
    STARTUPINFOW si = {0};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi;
    std::wstring cmd_line;
    cmd_line += L'"';
    cmd_line += exe_path;
    cmd_line += L'"';
    cmd_line += LR"( "foo" "バー" "æøå")";
    // Make room for null-terminating character
    cmd_line.resize(cmd_line.size() + 1);
    REQUIRE(CreateProcessW(exe_path.c_str(), cmd_line.data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi));
    if (WaitForSingleObject(pi.hProcess, 5000) != WAIT_OBJECT_0) {
      TerminateProcess(pi.hProcess, 1);
    }
    DWORD native_exit_code = 0;
    REQUIRE(GetExitCodeProcess(pi.hProcess, &native_exit_code));
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    exit_code = static_cast<int>(native_exit_code);
  #endif
  }
  return exit_code;
}
