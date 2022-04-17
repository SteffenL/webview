#ifdef WEBVIEW_BUILDING
#error WEBVIEW_BUILDING must not be defined
#endif

#define YES "yes"
#define NO "no"

#include "webview.h"

#if defined(WEBVIEW_SHARED)
#define LIBRARY_TYPE "shared"
#elif defined(WEBVIEW_STATIC)
#define LIBRARY_TYPE "static"
#else
#define LIBRARY_TYPE "header-only"
#endif

#ifdef WEBVIEW_INCLUDE_IMPLEMENTATION
#define IMPL_INCLUDED YES
#else
#define IMPL_INCLUDED NO
#endif

#ifdef WEBVIEW_HEADER
#define IMPL_OPT_OUT YES
#else
#define IMPL_OPT_OUT NO
#endif

#include <iostream>

int main() {
  webview_destroy(webview_create(0, nullptr));
  std::cout << "Type: " LIBRARY_TYPE << std::endl;
  std::cout << "Implementation included: " IMPL_INCLUDED << std::endl;
  std::cout << "Implementation opt-out: " IMPL_OPT_OUT << std::endl;
  return 0;
}
