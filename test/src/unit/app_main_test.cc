// =================================================================
// TEST: ensure that webview_app_main works.
// =================================================================

#define WEBVIEW_DEFINE_MAIN
#include "webview/test.h"
#include "webview/webview.h"

int webview_app_main(int argc, char *argv[]) {
  REQUIRE(argc == 2);
  REQUIRE(argv);
  REQUIRE(argv[0]);
  REQUIRE(argv[1]);
  return 0;
}
