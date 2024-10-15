#include <string>

constexpr auto* html_start{R"html(
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
      button { background: #e6e6e6; color: #333; border: .1em solid #32323226; border-radius: .5em; padding: .25em 1em; }
      button:active { background: #c3c3c3; }
      button:focus { outline: .1em solid #575757; outline-offset: -.25em; }
    </style>
  </head>
  <body>
)html"};

constexpr auto* html_end{R"html(
  </body>
</html>
)html"};

constexpr auto* main_window_html{R"html(
<main>
  <div>
    <h2>Window</h2>
    <button data-#click="cmdNewWindow">New window</button>
    <button data-#click="cmdRequestClose">Close?</button>
    <button data-#click="cmdClose">Close!</button>
  </div>
  <div>
    <h2>Fullscreen</h2>
    <button data-#click="cmdFullscreen">Fullscreen</button>
    <button data-#click="cmdUnfullscreen">Unfullscreen</button>
  </div>
</main>
<script>
  document.querySelectorAll("button[data-\\#click]").forEach(e => {
    e.addEventListener("click", () => {
      const cmd = e.dataset["#click"];
      console.log(cmd);
    });
  });
</script>
)html"};

constexpr auto *sub_window_html{R"html(
<main>
  <p>Sub window.</p>
</main>
)html"};

inline std::string make_html(const std::string& html) {
  return html_start + html + html_end;
}
