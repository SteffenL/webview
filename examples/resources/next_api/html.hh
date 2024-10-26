#include <string>

constexpr auto *html_start{R"html(
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <meta http-equiv="X-UA-Compatible" content="ie=edge" />
    <style>
      * { margin: 0; padding: 0; }
      body {
          background: #191919; color: #8e8e8e; cursor: default;
          font-family: -apple-system, system-ui, "Segoe UI", Helvetica, Arial, sans-serif, "Apple Color Emoji", "Segoe UI Emoji";
          -webkit-user-select: none; -ms-user-select: none; user-select: none; }
      button { background: #e6e6e6; color: #333; border: .1em solid #32323226; border-radius: .5em; padding: .25em 1em; white-space: pre; }
      button:active { background: #c3c3c3; }
      button:focus { outline: .1em solid #575757; outline-offset: -.25em; }
      /*h1, h2, h3, h4, h5, h6 { margin: 0 0 .5em 0; }*/
      .hstack { display: flex; flex-direction: row; }
      .vstack { display: flex; flex-direction: column; }
      .pad { padding: .5em; }
      .margin { margin: .5em; }
    </style>
  </head>
  <body>
)html"};

constexpr auto *html_end{R"html(
  </body>
</html>
)html"};

constexpr auto *main_window_html{R"html(
<main class="vstack">
  <div class="vstack">
    <div>
      <h2>Window</h2>
      <div class="hstack">
        <button data-#bind="click:cmdNewWindow">New window</button>
        <button data-#bind="click:cmdCloseWindow">Close window</button>
      </div>
    </div>
  </div>
  <div class="vstack">
    <h2>Fullscreen</h2>
    <div class="hstack">
      <button data-#bind="click:cmdFullscreen">Fullscreen</button>
      <button data-#bind="click:cmdUnfullscreen">Unfullscreen</button>
    </div>
  </div>
  <div class="vstack">
    <h2>Promise</h2>
    <div class="hstack">
      <button data-#on="click:cmdPromise">Run</button>
      <span id="promiseReplyValue"></span>
    </div>
  </div>
</main>
<script type="module">
  const funcs = {
    async cmdPromise() {
      promiseReplyValue.textContent = "(pending)";
      const res = await __webview__.call("cmdPromise");
      promiseReplyValue.textContent = res;
    }
  };
  window.addEventListener("contextmenu", e => e.preventDefault(), false);
  document.querySelectorAll("*[data-\\#bind]").forEach(element => {
    const [event, name] = element.dataset["#bind"].split(":");
    element.addEventListener(event, () => __webview__.call(name));
  });
  document.querySelectorAll("*[data-\\#on]").forEach(element => {
    const [event, name] = element.dataset["#on"].split(":");
    element.addEventListener(event, () => funcs[name]());
  });
</script>
)html"};

constexpr auto *sub_window_html{R"html(
<main>
  <p>Sub window.</p>
</main>
)html"};

inline std::string make_html(const std::string &html) {
  return html_start + html + html_end;
}
