#define WEBVIEW_DEFINE_MAIN
#include "webview/webview.h"

int webview_app_main(int argc, char *argv[]) {
  webview::webview w(false, nullptr);
  w.set_title("Basic Example");
  w.set_size(480, 320, WEBVIEW_HINT_NONE);
  w.set_html("Thanks for using webview!");
  w.run();
  return 0;
}
