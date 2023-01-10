#include "test.hh"
#include "webview.h"

void cb_assert_arg(webview_t w, void *arg) {
  assert(w != nullptr);
  assert(memcmp(arg, "arg", 3) == 0);
}

void cb_terminate(webview_t w, void *arg) {
  assert(arg == nullptr);
  webview_terminate(w);
}

// TEST: use C API to create a window, run app and terminate it.
TEST(c_api) {
  webview_t w;
  w = webview_create(false, nullptr);
  webview_set_size(w, 480, 320, 0);
  webview_set_title(w, "Test");
  webview_navigate(w, "https://github.com/zserge/webview");
  webview_dispatch(w, cb_assert_arg, (void *)"arg");
  webview_dispatch(w, cb_terminate, nullptr);
  webview_run(w);
  webview_destroy(w);
}