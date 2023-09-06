#include "webview.h"
#include <stddef.h>

static const char html[] =
    "<button id=\"tap\">Tap me</button>\n"
    "<script>\n"
    "  const tapElement = document.querySelector(\"#tap\");\n"
    "  document.addEventListener(\"DOMContentLoaded\", () => {\n"
    "    tapElement.addEventListener(\"click\", () => {\n"
    "      tap().then(result => {\n"
    "        // TODO\n"
    "      });\n"
    "    });\n"
    "  });\n"
    "</script>";

void on_tapped(const char *seq, const char *req, void *arg) {
  webview_t w = (webview_t)arg;
  webview_return(w, seq, 0, "");
}

int main() {
  webview_t w = webview_create(0, NULL);
  webview_set_title(w, "Example");
  webview_set_size(w, 480, 320, WEBVIEW_HINT_NONE);
  webview_bind(w, "tap", on_tapped, w);
  webview_set_html(w, html);
  webview_run(w);
  webview_destroy(w);
  return 0;
}
