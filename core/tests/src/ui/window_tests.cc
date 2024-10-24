#include "webview/test_driver.hh"

#include "webview/experimental/webview.hh"

#include <cassert>

using namespace webview;

TEST_CASE("window event: close_requested") {
  application app;

  window win{window_options{}.set_size({800, 600}).set_title("Hello")};
  win.set_visible(true);
  win.events().close_requested.bind([&] (window* sender) {
    sender->destroy();
    app.terminate();
  });

  app.dispatch([&] { app.terminate(); });
  app.run();
}
