#define WEBVIEW_VERSION_MAJOR 1
#define WEBVIEW_VERSION_MINOR 2
#define WEBVIEW_VERSION_PATCH 3
#define WEBVIEW_VERSION_PRE_RELEASE "-test"
#define WEBVIEW_VERSION_BUILD_METADATA "+gaabbccd"

#include "test.hpp"
#include "webview.h"

// TEST: webview_version().
TEST(c_api_version) {
  auto vi = webview_version();
  assert(vi);
  assert(vi->version.major == 1);
  assert(vi->version.minor == 2);
  assert(vi->version.patch == 3);
  assert(std::string(vi->version_number) == "1.2.3");
  assert(std::string(vi->pre_release) == "-test");
  assert(std::string(vi->build_metadata) == "+gaabbccd");
  // The function should return the same pointer when called again.
  assert(webview_version() == vi);
}
