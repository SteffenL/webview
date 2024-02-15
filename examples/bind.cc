#include "webview.h"

#include <list>

constexpr const auto html =
    R"html(<button id="quit-button">Quit</button><button id="new-window-button">New</button>
<script>
  (() => {
    const quitButton = document.getElementById("quit-button");
    const newWindowButton = document.getElementById("new-window-button");
    quitButton.addEventListener("click", () => { window.quit(); });
    newWindowButton.addEventListener("click", () => { window.newWindow(); });
  })();
</script>)html";

webview::webview *create_webview() {
    static std::list<std::unique_ptr<webview::webview>> instances;

    std::unique_ptr<webview::webview> w{new webview::webview{true, nullptr}};
    auto *wp = w.get();

    w->set_title("Multi-window Example");
    w->set_size(480, 320, WEBVIEW_HINT_NONE);

    w->bind("quit", [=](const std::string & /*req*/) -> std::string {
      wp->terminate();
      return "";
    });

    w->bind("newWindow", [&](const std::string & /*req*/) -> std::string {
      create_webview();
      return "";
    });

    w->set_html(html);

    instances.push_back(std::move(w));

    return wp;
}

int main() {
  create_webview()->run();
  return 0;
}
