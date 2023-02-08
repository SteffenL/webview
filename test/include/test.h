#ifndef WEBVIEW_TEST_H
#define WEBVIEW_TEST_H

#include <cstdio>
#include <cstdlib>

#define REQUIRE(condition)                                                     \
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
