#define WEBVIEW_DEFINE_MAIN
#include "webview/webview.h"

#include <stdio.h>
#include <stdlib.h>

void myFunc(const char *seq, const char *req, void *arg) {
  printf("Params: %s\n", req);
}

int webview_app_main(int argc, const char *argv[]) {
  webview_t w = webview_create(0, NULL);
  webview_set_title(w, "Basic Example");
  webview_set_size(w, 480, 320, WEBVIEW_HINT_NONE);
  webview_set_html(w, "Thanks for using webview!");
  webview_run(w);
  webview_destroy(w);
  return 0;
}
