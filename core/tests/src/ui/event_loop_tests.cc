#include "webview/test_driver.hh"

#include "webview/experimental/webview.hh"

#include <cassert>

using namespace webview;

TEST_CASE("event_loop::iterate()") {
  event_loop loop;
  int event1{};
  int event2{};
  loop.iterate(false);
  REQUIRE(event1 == 0);
  REQUIRE(event2 == 0);
  // Shouldn't iterate() process one event at a time?
  loop.dispatch([&] { event1 = 1; });
  loop.dispatch([&] { event2 = 2; });
  REQUIRE(event1 == 0);
  REQUIRE(event2 == 0);
  loop.iterate(true);
  REQUIRE(event1 == 1);
  REQUIRE(event2 == 2);
}

TEST_CASE("event_loop::run()") {
  event_loop loop;
  int event1{};
  loop.dispatch([&] {
    event1 = 1;
    loop.stop();
  });
  REQUIRE(event1 == 0);
  loop.run();
  REQUIRE(event1 == 1);
  loop.dispatch([&] {
    event1 = 2;
    loop.stop();
  });
  REQUIRE(event1 == 1);
  loop.run();
  REQUIRE(event1 == 2);
}
