/*
 * MIT License
 *
 * Copyright (c) 2017 Serge Zaitsev
 * Copyright (c) 2022 Steffen André Langnes
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef WEBVIEW_MAIN_H
#define WEBVIEW_MAIN_H

#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

int webview_main(int argc, const char *argv[]);

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
  int argc = 0;
  LPWSTR *wargv{::CommandLineToArgvW(::GetCommandLineW(), &argc)};
  if (!wargv) {
    abort();
  }
  if (argc == 0) {
    ::LocalFree(wargv);
    return webview_main(0, 0);
  }
  const UINT cp = CP_UTF8;
  const DWORD flags = 0x00000080U /*WC_ERR_INVALID_CHARS*/;
  char **argv = malloc(sizeof(char *) * argc);
  if (!argv) {
    abort();
  }
  for (int i = 0; i < argc; i++) {
    LPWSTR warg = wargv[i];
    size_t warg_length = wcslen(warg);
    if (warg_length > 0) {
      int arg_length = WideCharToMultiByte(cp, flags, warg, warg_length, NULL,
                                           0, NULL, NULL);
      if (arg_length == 0) {
        abort();
      }
      char *arg = malloc(arg_length);
      if (!arg) {
        abort();
      }
      if (WideCharToMultiByte(cp, flags, warg, warg_length, arg, arg_length,
                              NULL, NULL) == 0) {
        abort();
      }
      arg[arg_length] = '\0';
      argv[i] = arg;
    } else {
      char *arg = malloc(1);
      if (!arg) {
        abort();
      }
      arg[0] = '\0';
      argv[i] = arg;
    }
  }
  ::LocalFree(wargv);
  int exit_code = webview_main(argc, argv);
  for (int i = 0; i < argc; i++) {
    free(argv[i]);
  }
  free(argv);
  return exit_code;
}
#else
int main(int argc, const char *argv[]) { return webview_main(argc, argv); }
#endif

#endif // WEBVIEW_MAIN_H
