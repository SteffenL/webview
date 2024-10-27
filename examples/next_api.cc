#include "resources/next_api/html.hh"
#include "webview/detail/thread.hh"
#include "webview/experimental/webview.hh"

#include <chrono>
#include <list>
#include <memory>
#include <thread>
#include <unordered_map>

class sub_window {
public:
  sub_window() {
    w.browser()->set_html(make_html(sub_window_html));
    w.set_visible(true);
  }

private:
  webview::window w{
      webview::window_options{}.set_size({480, 320}).set_title("Sub Window")};
};

class main_window {
public:
  main_window() {
    w.events().close_requested.bind([&](webview::iwindow *sender) {
      sender->destroy();
      return true;
    });
    w.events().destroy.bind([&](webview::iwindow * /*sender*/) {
      webview::current_application().terminate();
      return true;
    });
    add_bindings();
    w.browser()->set_html(make_html(main_window_html));
    w.set_visible(true);
  }

private:
  void add_bindings() {
    auto bridge{w.browser()->bridge()};
    bridge->bind("cmdNewWindow", [=] {
      std::shared_ptr<sub_window> ptr{new sub_window{}};
      auto it{m_sub_windows.insert(m_sub_windows.end(), ptr)};
      m_sub_window_map.emplace(ptr.get(), it);
    });
    bridge->bind("cmdCloseWindow", [=] { w.close(); });
    bridge->bind("cmdFullscreen", [=] { w.set_fullscreen(true); });
    bridge->bind("cmdUnfullscreen", [=] { w.set_fullscreen(false); });
    bridge->bind("cmdTasks", [=](webview::binding_arg &arg) {
      m_task_thread = webview::detail::thread{
          [](webview::binding_promise promise) {
            std::this_thread::sleep_for(std::chrono::seconds{1});
            promise.resolve("\"hello\"");
          },
          arg.get_promise()};
    });
  }

  webview::window w{
      webview::window_options{}.set_size({480, 320}).set_title("Main Window")};
  std::list<std::shared_ptr<sub_window>> m_sub_windows;
  std::unordered_map<sub_window *, typename decltype(m_sub_windows)::iterator>
      m_sub_window_map;
  webview::detail::thread m_task_thread;
};

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
#else
int main() {
#endif
  webview::application app;
  main_window main_window;
  app.run();
  return 0;
}
