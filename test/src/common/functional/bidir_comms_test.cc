// =================================================================
// TEST: ensure that JS code can call native code and vice versa.
// =================================================================

#include "webview/webview.h"
#include "webview/test.h"

#include <cstring>
#include <iostream>

struct test_webview : webview::browser_engine {
  using cb_t = std::function<void(test_webview *, int, const std::string &)>;
  test_webview(cb_t cb) : webview::browser_engine(true, nullptr), m_cb(cb) {}
  void on_message(const std::string &msg) override { m_cb(this, i++, msg); }
  int i = 0;
  cb_t m_cb;
};

int main() {
  test_webview browser([](test_webview *w, int i, const std::string &msg) {
    std::cout << msg << std::endl;
    switch (i) {
    case 0:
      REQUIRE(msg == "loaded");
      w->eval("window.external.invoke('exiting ' + window.x)");
      break;
    case 1:
      REQUIRE(msg == "exiting 42");
      w->terminate();
      break;
    default:
      REQUIRE(0);
    }
  });
  browser.init(R"(
    window.x = 42;
    window.onload = () => {
      window.external.invoke('loaded');
    };
  )");
  browser.navigate("data:text/html,%3Chtml%3Ehello%3C%2Fhtml%3E");
  browser.run();
  return 0;
}
