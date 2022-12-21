// =================================================================
// TEST: test synchronous binding and unbinding.
// =================================================================

#include "webview/webview.h"
#include "webview/test.h"

int main() {
  unsigned int number = 0;
  webview::webview w(false, nullptr);
  auto test = [&](const std::string &req) -> std::string {
    auto increment = [&](const std::string & /*req*/) -> std::string {
      ++number;
      return "";
    };
    // Bind and increment number.
    if (req == "[0]") {
      REQUIRE(number == 0);
      w.bind("increment", increment);
      return "(() => {try{window.increment()}catch{}window.test(1)})()";
    }
    // Unbind and make sure that we cannot increment even if we try.
    if (req == "[1]") {
      REQUIRE(number == 1);
      w.unbind("increment");
      return "(() => {try{window.increment()}catch{}window.test(2)})()";
    }
    // Number should not have changed but we can bind again and change the number.
    if (req == "[2]") {
      REQUIRE(number == 1);
      w.bind("increment", increment);
      return "(() => {try{window.increment()}catch{}window.test(3)})()";
    }
    // Finish test.
    if (req == "[3]") {
      REQUIRE(number == 2);
      w.terminate();
      return "";
    }
    REQUIRE(!"Should not reach here");
    return "";
  };
  auto html = "<script>\n"
              "  window.test(0);\n"
              "</script>";
  // Attempting to remove non-existing binding is OK
  w.unbind("test");
  w.bind("test", test);
  // Attempting to bind multiple times only binds once
  w.bind("test", test);
  w.set_html(html);
  w.run();
  return 0;
}
