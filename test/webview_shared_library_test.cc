#ifdef WEBVIEW_BUILDING
#error WEBVIEW_BUILDING must not be defined
#endif

#ifdef WEBVIEW_HEADER
#error WEBVIEW_HEADER must not be defined
#endif

#ifdef WEBVIEW_STATIC
#error WEBVIEW_STATIC must not be defined
#endif

#ifndef WEBVIEW_SHARED
#error WEBVIEW_SHARED must be defined
#endif

#include "webview.h"

#include <iostream>

int main() {
  auto w = webview_create(false, nullptr);
  webview_destroy(w);
  std::cout << "OK" << std::endl;
  return 0;
}
