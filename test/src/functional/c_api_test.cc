// =================================================================
// TEST: use C API to create a window, run app and terminate it.
// =================================================================

#include "webview/webview.h"
#include "webview/test.h"

#include <cstring>

void cb_assert_arg(webview_t w, void *arg) {
  REQUIRE(w != nullptr);
  REQUIRE(memcmp(arg, "arg", 3) == 0);
}

void cb_terminate(webview_t w, void *arg) {
  REQUIRE(arg == nullptr);
  webview_terminate(w);
}

int main() {
  webview_t w;
  w = webview_create(false, nullptr);
  webview_set_size(w, 480, 320, 0);
  webview_set_title(w, "Test");
  webview_navigate(w, "https://github.com/zserge/webview");
  webview_dispatch(w, cb_assert_arg, (void *)"arg");
  webview_dispatch(w, cb_terminate, nullptr);
  webview_run(w);
  webview_destroy(w);
  return 0;
}
