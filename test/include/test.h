#ifndef WEBVIEW_TEST_H
#define WEBVIEW_TEST_H

#include <cassert>
#include <cstdio>
#include <cstdlib>

// Redefine assert() macro so that tests work in release builds.

#ifdef assert
#undef assert
#endif

#define assert(condition)                                                      \
  if (!(condition)) {                                                          \
    std::printf(                                                               \
        "############################################################\n"       \
        "Assertion failed: %s\n"                                               \
        "Line: %u\n"                                                           \
        "File: %s\n"                                                           \
        "############################################################\n",      \
        #condition, __LINE__, __FILE__);                                       \
    std::exit(1);                                                              \
  }

#endif // WEBVIEW_TEST_H
