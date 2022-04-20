// =================================================================
// TEST: ensure that JSON parsing works.
// =================================================================

#include "webview/webview.h"
#include "test.h"

int main() {
  auto J = webview::json_parse;
  REQUIRE(J(R"({"foo":"bar"})", "foo", -1) == "bar");
  REQUIRE(J(R"({"foo":""})", "foo", -1) == "");
  REQUIRE(J(R"({"foo": {"bar": 1}})", "foo", -1) == R"({"bar": 1})");
  REQUIRE(J(R"(["foo", "bar", "baz"])", "", 0) == "foo");
  REQUIRE(J(R"(["foo", "bar", "baz"])", "", 2) == "baz");
  return 0;
}
