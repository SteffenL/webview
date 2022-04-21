#define WEBVIEW_DEFINE_MAIN
#include "webview/webview.h"

#include <iostream>

constexpr const auto html = R"|||(
<!DOCTYPE html>
<html>
  <head>
    <title>webview example</title>
  </head>
  <body>
    <p>Your user agent is as follows.</p>
    <p><code id="userAgent"></code></p>
    <p>Try to ping.</p>
    <button id="ping">Ping</button>
    <p id="answer"></p>
    <script>
      document.getElementById("ping").addEventListener("click", function() {
        ping({ "message": "Ping!" }, 123).then(function(result) {
          console.log("Received pong:", JSON.stringify(result));
          document.getElementById("answer").textContent = result.message;
        });
      });
      document.getElementById("userAgent").textContent = navigator.userAgent;
    </script>
  </body>
</html>
)|||";

int webview_app_main(int argc, char *argv[]) {
  webview::webview w(true, nullptr);
  w.set_title("Basic example");
  w.set_size(480, 320, WEBVIEW_HINT_NONE);
  w.bind("ping", [](const std::string &s) -> std::string {
    std::cout << "Received ping: " << s << "\n";
    auto req_object = webview::json_parse(s, "", 0);
    auto req_array = webview::json_parse(s, "", 1);
    auto message = webview::json_parse(req_object, "message", -1);
    auto number = webview::json_parse(req_array, "", 0);
    return R"({"message": "Pong )" + number + R"(!"})";
  });
  w.set_html(html);
  w.run();
  return 0;
}
