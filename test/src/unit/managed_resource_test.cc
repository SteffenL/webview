// =================================================================
// TEST: ensure that RAII-ifying resources works.
// =================================================================

#include "webview/test.h"
#include "webview/webview.h"

int main() {
  using namespace webview::detail;
  bool deleted = false;
  {
    auto wrapped = wrap_resource(123, [&](auto value) {
      REQUIRE(value == 123);
      REQUIRE(!deleted);
      deleted = true;
    });
    REQUIRE(!deleted);
  }
  REQUIRE(deleted);
  return 0;
}
