// =================================================================
// TEST: ensure that JSON parsing works.
// =================================================================

#include "test.h"
#include "webview/webview.h"

int main() {
  auto J = webview::json_parse;
  // Valid input with expected output
  REQUIRE(J(R"({"foo":"bar"})", "foo", -1) == "bar");
  REQUIRE(J(R"({"foo":""})", "foo", -1) == "");
  REQUIRE(J(R"({"foo":{}")", "foo", -1) == "{}");
  REQUIRE(J(R"({"foo": {"bar": 1}})", "foo", -1) == R"({"bar": 1})");
  REQUIRE(J(R"(["foo", "bar", "baz"])", "", 0) == "foo");
  REQUIRE(J(R"(["foo", "bar", "baz"])", "", 2) == "baz");
  // Valid UTF-8 with expected output
  REQUIRE(J(u8R"({"フー":"バー"})", "フー", -1) == u8"バー");
  REQUIRE(J(u8R"(["フー", "バー", "バズ"])", "", 2) == u8"バズ");
  // Invalid input with valid output - should probably fail
  REQUIRE(J(R"({"foo":"bar")", "foo", -1) == "bar");
  // Valid input with other invalid parameters - should fail
  REQUIRE(J(R"([])", "", 0) == "");
  REQUIRE(J(R"({})", "foo", -1) == "");
  REQUIRE(J(R"(["foo", "bar", "baz"])", "", -1) == "");
  REQUIRE(J(R"(["foo"])", "", 1234) == "");
  REQUIRE(J(R"(["foo"])", "", -1234) == "");
  // Invalid input - should fail
  REQUIRE(J(R"()", "", 0) == "");
  REQUIRE(J(R"()", "foo", -1) == "");
  REQUIRE(J(R"({"foo":")", "foo", -1) == "");
  REQUIRE(J(R"({"foo":{)", "foo", -1) == "");
  REQUIRE(J(R"({"foo":{")", "foo", -1) == "");
  REQUIRE(J(R"(}")", "foo", -1) == "");
  REQUIRE(J(R"({}}")", "foo", -1) == "");
  REQUIRE(J(R"("foo)", "foo", -1) == "");
  REQUIRE(J(R"(foo)", "foo", -1) == "");
  REQUIRE(J(R"({{[[""foo""]]}})", "", 1234) == "");
  return 0;
}
