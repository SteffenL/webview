#include "test.hpp"
#include "webview.h"

// TEST: ensure that JSON parsing works.
TEST(json) {
  auto J = webview::detail::json_parse;
  // Valid input with expected output
  assert(J(R"({"foo":"bar"})", "foo", -1) == "bar");
  assert(J(R"({"foo":""})", "foo", -1).empty());
  assert(J(R"({"foo":{}")", "foo", -1) == "{}");
  assert(J(R"({"foo": {"bar": 1}})", "foo", -1) == R"({"bar": 1})");
  assert(J(R"(["foo", "bar", "baz"])", "", 0) == "foo");
  assert(J(R"(["foo", "bar", "baz"])", "", 2) == "baz");
  // Valid UTF-8 with expected output
  assert(J(R"({"フー":"バー"})", "フー", -1) == "バー");
  assert(J(R"(["フー", "バー", "バズ"])", "", 2) == "バズ");
  // Invalid input with valid output - should probably fail
  assert(J(R"({"foo":"bar")", "foo", -1) == "bar");
  // Valid input with other invalid parameters - should fail
  assert(J(R"([])", "", 0).empty());
  assert(J(R"({})", "foo", -1).empty());
  assert(J(R"(["foo", "bar", "baz"])", "", -1).empty());
  assert(J(R"(["foo"])", "", 1234).empty());
  assert(J(R"(["foo"])", "", -1234).empty());
  // Invalid input - should fail
  assert(J("", "", 0).empty());
  assert(J("", "foo", -1).empty());
  assert(J(R"({"foo":")", "foo", -1).empty());
  assert(J(R"({"foo":{)", "foo", -1).empty());
  assert(J(R"({"foo":{")", "foo", -1).empty());
  assert(J(R"(}")", "foo", -1).empty());
  assert(J(R"({}}")", "foo", -1).empty());
  assert(J(R"("foo)", "foo", -1).empty());
  assert(J(R"(foo)", "foo", -1).empty());
  assert(J(R"({{[[""foo""]]}})", "", 1234).empty());
  assert(J("bad", "", 0).empty());
  assert(J("bad", "foo", -1).empty());
}
