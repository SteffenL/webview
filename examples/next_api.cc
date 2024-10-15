#include "webview/experimental/ui.hh"

#include <list>
#include <memory>

constexpr auto *main_window_html{R"html(
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <meta http-equiv="X-UA-Compatible" content="ie=edge" />
    <style>
      * { margin: 0; padding: 0; }
      body { background: #191919; color: #8e8e8e; font-family: -apple-system, system-ui, "Segoe UI", Helvetica, Arial, sans-serif, "Apple Color Emoji", "Segoe UI Emoji"; }
      button { background: #e6e6e6; color: #333; border: .1em solid #32323226; border-radius: .5em; padding: .25em 1em; }
      button:active { background: #c3c3c3; }
      button:focus { outline: .1em solid #575757; outline-offset: -.25em; }
    </style>
  </head>
  <body>
    <main>
      <div>
        <h2>Window</h2>
        <button data-#click="cmdNewWindow">New window</button>
        <button data-#click="cmdClose">Close</button>
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
  </body>
</html>
)html"};

constexpr auto *sub_window_html{R"html(
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <meta http-equiv="X-UA-Compatible" content="ie=edge" />
    <style>
      * { margin: 0; padding: 0; }
      body { background: #191919; color: #8e8e8e; font-family: -apple-system, system-ui, "Segoe UI", Helvetica, Arial, sans-serif, "Apple Color Emoji", "Segoe UI Emoji"; }
    </style>
  </head>
  <body>
    <main>
      <p>Sub window.</p>
    </main>
  </body>
</html>
)html"};

class sub_window {
public:
  sub_window() : m_window{webview::window_options{}.set_title("Sub Window")} {
    m_window.get_widget().set_html(sub_window_html);
    m_window.set_visible(true);
  }

private:
  webview::window m_window;
};

class main_window {
public:
  main_window() : m_window{webview::window_options{}.set_title("Main Window")} {
    m_window.get_widget().set_html(main_window_html);
    m_window.set_visible(true);
  }

  std::shared_ptr<sub_window> new_window() {
    auto sub{std::shared_ptr<sub_window>{new sub_window{}}};
    m_sub_windows.push_back(sub);
    return sub;
  }

private:
  webview::window m_window;
  std::list<std::shared_ptr<sub_window>> m_sub_windows;
};

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE /*hInst*/, HINSTANCE /*hPrevInst*/,
                   LPSTR /*lpCmdLine*/, int /*nCmdShow*/) {
#else
int main() {
#endif
  webview::application app;
  main_window window;
  app.run();
  return 0;
}
