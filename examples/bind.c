#include "webview.h"

static const char html[] = "\
<button id=\"quit\">Quit</button>\n\
<input type=\"file\" />\n\
<script type=\"module\">\n\
  document.getElementById(\"quit\").addEventListener(\"click\", async () => {\n\
    await window.quit();\n\
  });\n\
</script>";

void quit(const char *id, const char *req, void *arg) {
  webview_t *w = (webview_t *)arg;
  webview_terminate(w);
}

int main(void) {
  webview_t w = webview_create(0, 0);
  webview_set_title(w, "Bind Example");
  webview_set_size(w, 480, 320, WEBVIEW_HINT_NONE);
  webview_bind(w, "quit", quit, w);
  webview_set_html(w, html);
  webview_run(w);
  webview_destroy(w);
  return 0;
}