// =================================================================
// TEST: start app loop and terminate it.
// =================================================================

#define WEBVIEW_DEFINE_CONSOLE_MAIN
#include "webview/webview.h"

int webview_app_main(int argc, char *argv[]) {
  webview::webview w(false, nullptr);
  w.dispatch([&]() { w.terminate(); });
  w.run();
  return 0;
}
