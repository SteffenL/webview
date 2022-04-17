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
  webview_destroy(webview_create(0, nullptr));
  std::cout << "OK" << std::endl;
  return 0;
}
