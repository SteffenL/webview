#define WEBVIEW_DEFINE_MAIN
#include "webview/webview.h"

constexpr const auto html =
    R"html(<button id="increment">Tap me</button>
<div>You tapped <span id="count">0</span> time(s).</div>
<script>
  const [incrementElement, countElement] =
    document.querySelectorAll("#increment, #count");
  document.addEventListener("DOMContentLoaded", () => {
    incrementElement.addEventListener("click", () => {
      window.increment().then(result => {
        countElement.textContent = result.count;
      });
    });
  });
</script>)html";

int webview_app_main(int argc, const char *argv[]) {
  unsigned int count = 0;
  webview::webview w(false, nullptr);
  w.set_title("Bind Example");
  w.set_size(480, 320, WEBVIEW_HINT_NONE);
  w.bind("increment", [&](const std::string &s) -> std::string {
    auto count_string = std::to_string(++count);
    return "{\"count\": " + count_string + "}";
  });
  w.set_html(html);
  w.run();
  return 0;
}
