// =================================================================
// TEST: start app loop and terminate it.
// =================================================================

#include "webview/webview.h"

int main() {
  webview::webview w(false, nullptr);
  w.dispatch([&]() { w.terminate(); });
  w.run();
  return 0;
}
