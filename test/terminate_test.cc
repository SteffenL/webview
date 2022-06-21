// +build ignore

#include "webview.h"

// =================================================================
// TEST: start app loop and terminate it.
// =================================================================
int terminate_test(int, char *[]) {
  webview::webview w(false, nullptr);
  w.dispatch([&]() { w.terminate(); });
  w.run();
  return 0;
}
