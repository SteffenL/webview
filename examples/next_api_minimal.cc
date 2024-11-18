#include "webview/experimental/main.h"
#include "webview/experimental/webview.hh"

int webview_main(int /*argc*/, const char * /*argv*/[]) {
  webview::application app;
  webview::window window{webview::window_options{}
                             .set_size({480, 320})
                             .set_title("Minimal Example")
                             .set_terminate_on_destroy(true)};
  window.browser()->set_html("Hello");
  window.set_visible(true);
  app.run();
  return 0;
}
