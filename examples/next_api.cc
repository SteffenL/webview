#include "resources/next_api/html.hh"
#include "webview/experimental/ui.hh"

#include <iostream>
#include <list>
#include <memory>
#include <unordered_map>

class sub_window {
public:
  sub_window() : m_inner{webview::window_options{}.set_title("Sub Window")} {
    m_inner.get_widget().set_html(make_html(sub_window_html));
    m_inner.set_visible(true);
  }

private:
  webview::window m_inner;
};

class main_window {
public:
  main_window() : m_inner{webview::window_options{}.set_title("Main Window")} {
    m_inner.events().close_requested.bind(
        [] { std::cout << "Close requested\n"; });
    m_inner.get_widget().set_html(make_html(main_window_html));
    m_inner.set_visible(true);
  }

private:
  void cmd_new_window() {
    std::shared_ptr<sub_window> ptr{new sub_window{}};
    auto it{m_sub_windows.insert(m_sub_windows.end(), ptr)};
    m_sub_window_map.emplace(ptr.get(), it);
  }

  //void cmd_request_close() { m_inner.destroy(); }
  void cmd_close() { m_inner.close(); }
  void cmd_fullscreen() {}
  void cmd_unfullscreen() {}

  webview::window m_inner;
  std::list<std::shared_ptr<sub_window>> m_sub_windows;
  std::unordered_map<sub_window *, typename decltype(m_sub_windows)::iterator>
      m_sub_window_map;
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
