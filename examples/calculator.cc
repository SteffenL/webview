#define WEBVIEW_DEFINE_MAIN
#include "webview/webview.h"

#include <iostream>

constexpr const auto html = R"|||(
<!DOCTYPE html>
<html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>webview example</title>
    <style>
      html {
        font-size: 10px;
        margin: 0;
      }
      body {
        margin: 0;
        padding: 1rem;
      }
      * {
        font-size: 5rem;
      }
      input {
        min-width: 0;
      }
      .vstack, .hstack {
        display: flex;
        gap: 1rem;
      }
      .hstack {
        flex-direction: row;
      }
      .vstack {
        flex-direction: column;
      }
      .hstack > .grow {
        flex-grow: 1;
      }
      .text-center {
        text-align: center;
      }
      .text-left {
        text-align: left;
      }
      .text-right {
        text-align: right;
      }
      .divider {
        border-bottom: .2rem solid #000;
      }
    </style>
    <script>
      document.addEventListener("DOMContentLoaded", function() {
        var lhsElement = document.getElementById("lhs");
        var rhsElement = document.getElementById("rhs");
        var answerElement = document.getElementById("answer");

        var calculate = function() {
          var lhs = new Number(lhsElement.value);
          var rhs = new Number(rhsElement.value);
          add(lhs, rhs).then(function(res) {
            answerElement.textContent = res.result;
          });
        };

        lhsElement.addEventListener("input", calculate);
        rhsElement.addEventListener("input", calculate);
        calculate();
      });
    </script>
  </head>
  <body class="text-center">
    <div class="vstack">
      <div class="hstack">
        <input class="grow text-right" id="lhs" type="text" value="1" />
        <span>+</span>
        <input class="grow text-left" id="rhs" type="text" value="2" />
      </div>
      <div class="divider"></div>
      <div class="answer" id="answer"></div>
    </div>
  </body>
</html>
)|||";

int webview_app_main(int argc, char *argv[]) {
  webview::webview w(true, nullptr);
  w.set_title("Calculation");
  w.set_size(480, 320, WEBVIEW_HINT_NONE);
  w.bind("add", [](const std::string &s) -> std::string {
    try {
      auto lhs = std::stoi(webview::json_parse(s, "", 0));
      auto rhs = std::stoi(webview::json_parse(s, "", 1));
      return R"({"result": ")" + std::to_string(lhs + rhs) + R"("})";
    } catch (const std::exception &) {
      return R"({"result": "error"})";
    }
  });
  w.set_html(html);
  w.run();
  return 0;
}
