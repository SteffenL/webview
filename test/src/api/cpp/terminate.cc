#include "test.hpp"
#include "webview.h"

// TEST: start app loop and terminate it.
TEST(terminate) {
  webview::webview w(false, nullptr);
  w.dispatch([&]() { w.terminate(); });
  w.run();
}
