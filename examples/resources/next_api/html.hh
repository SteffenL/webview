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
      button:disabled { background: #c3c3c3; outline: none; opacity: .5; cursor: not-allowed; }
      .hstack { display: flex; flex-direction: row; flex-wrap: nowrap; }
      .vstack { display: flex; flex-direction: column; flex-wrap: nowrap; }
      .gap { gap: .5em; }
      .pad { padding: .5em; }
    </style>
  </head>
  <body class="vstack gap pad">
)html"};

constexpr auto *html_end{R"html(
  </body>
</html>
)html"};

constexpr auto *main_window_html{R"html(
<h2>Window</h2>
<div class="hstack gap">
  <button data-#bind="click:newWindow">New window</button>
  <button data-#bind="click:closeWindow">Close window</button>
</div>
<h2>Fullscreen</h2>
<div class="hstack gap">
  <button data-#bind="click:fullscreen">Fullscreen</button>
  <button data-#bind="click:unfullscreen">Unfullscreen</button>
</div>
<h2>Tasks</h2>
<div class="hstack gap">
  <button data-#on="click:runTasks">Run</button>
  <span id="tasksReply"></span>
</div>
<script type="module">
  const funcs = {
    async runTasks(event) {
      try {
        event.target.disabled = true;
        tasksReply.textContent = await __webview__.call("runTasks");
      } finally {
        event.target.disabled = false;
      }
    }
  };
  window.addEventListener("contextmenu", e => e.preventDefault(), false);
  document.querySelectorAll("*[data-\\#bind]").forEach(element => {
    const [event, name] = element.dataset["#bind"].split(":");
    element.addEventListener(event, () => __webview__.call(name));
  });
  document.querySelectorAll("*[data-\\#on]").forEach(element => {
    const [event, name] = element.dataset["#on"].split(":");
    element.addEventListener(event, (...args) => funcs[name](...args));
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
