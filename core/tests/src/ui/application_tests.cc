#include "webview/test_driver.hh"

#include "webview/experimental/webview.hh"

#include <cassert>

using namespace webview;

TEST_CASE("application::run()") {
  int event1{};
  application app;
  app.dispatch([&] {
    event1 = 1;
    app.terminate();
  });
  REQUIRE(event1 == 0);
  app.run();
  REQUIRE(event1 == 1);
}
