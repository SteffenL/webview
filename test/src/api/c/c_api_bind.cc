#include "test.hh"
#include "webview.h"

// TEST: use C API to test binding and unbinding.
TEST(c_api_bind) {
  struct context_t {
    webview_t w;
    unsigned int number;
  } context{};
  auto test = +[](const char *seq, const char *req, void *arg) {
    auto increment =
        +[](const char * /*seq*/, const char * /*req*/, void *arg) {
          ++static_cast<context_t *>(arg)->number;
        };
    auto context = static_cast<context_t *>(arg);
    std::string req_(req);
    // Bind and increment number.
    if (req_ == "[0]") {
      assert(context->number == 0);
      webview_bind(context->w, "increment", increment, context);
      webview_return(
          context->w, seq, 0,
          "(() => {try{window.increment()}catch{}window.test(1)})()");
      return;
    }
    // Unbind and make sure that we cannot increment even if we try.
    if (req_ == "[1]") {
      assert(context->number == 1);
      webview_unbind(context->w, "increment");
      webview_return(
          context->w, seq, 0,
          "(() => {try{window.increment()}catch{}window.test(2)})()");
      return;
    }
    // Number should not have changed but we can bind again and change the number.
    if (req_ == "[2]") {
      assert(context->number == 1);
      webview_bind(context->w, "increment", increment, context);
      webview_return(
          context->w, seq, 0,
          "(() => {try{window.increment()}catch{}window.test(3)})()");
      return;
    }
    // Finish test.
    if (req_ == "[3]") {
      assert(context->number == 2);
      webview_terminate(context->w);
      return;
    }
    assert(!"Should not reach here");
  };
  auto html = "<script>\n"
              "  window.test(0);\n"
              "</script>";
  auto w = webview_create(false, nullptr);
  context.w = w;
  // Attempting to remove non-existing binding is OK
  webview_unbind(w, "test");
  webview_bind(w, "test", test, &context);
  // Attempting to bind multiple times only binds once
  webview_bind(w, "test", test, &context);
  webview_set_html(w, html);
  webview_run(w);
}
