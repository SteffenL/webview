#define WEBVIEW_VERSION_MAJOR 1
#define WEBVIEW_VERSION_MINOR 2
#define WEBVIEW_VERSION_PATCH 3
#define WEBVIEW_VERSION_PRE_RELEASE "-test"
#define WEBVIEW_VERSION_BUILD_METADATA "+gaabbccd"

// =================================================================
// TEST: webview_version().
// =================================================================

#include "webview/webview.h"
#include "webview/test.h"

#include <string>

int main() {
  auto vi = webview_version();
  REQUIRE(vi);
  REQUIRE(vi->version.major == 1);
  REQUIRE(vi->version.minor == 2);
  REQUIRE(vi->version.patch == 3);
  REQUIRE(std::string(vi->version_number) == "1.2.3");
  REQUIRE(std::string(vi->pre_release) == "-test");
  REQUIRE(std::string(vi->build_metadata) == "+gaabbccd");
  // The function should return the same pointer when called again.
  REQUIRE(webview_version() == vi);
  return 0;
}
