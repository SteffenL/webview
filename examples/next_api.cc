#include "resources/next_api/html.hh"
#include "webview/experimental/webview.hh"

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
#else
int main() {
#endif
  webview::application app;
  webview::window_manager wm;
  webview::task_executor tasks;

  auto main{wm.new_window(
      webview::window_options{}.set_size({480, 320}).set_title("Main Window"))};
  auto main_bridge{main->browser()->bridge()};
  main_bridge->bind("newWindow", [&wm] {
    auto sub{wm.new_window(webview::window_options{}
                               .set_size({480, 320})
                               .set_title("Sub Window"))};
    sub->browser()->bridge()->bind("closeWindow", [=] { sub->close(); });
    sub->browser()->set_html(make_html(sub_window_html));
    sub->set_visible(true);
  });
  main_bridge->bind("closeWindow", [=] { main->close(); });
  main_bridge->bind("fullscreen", [=] { main->set_fullscreen(true); });
  main_bridge->bind("unfullscreen", [=] { main->set_fullscreen(false); });
  main_bridge->bind("runTasks", [&tasks](webview::binding_arg &arg) {
    tasks.put(
        [](webview::binding_promise promise) {
          static std::atomic_int i{};
          std::this_thread::sleep_for(std::chrono::seconds{1});
          promise.resolve(std::to_string(++i));
        },
        arg.get_promise());
  });
  main->browser()->set_html(make_html(main_window_html));
  main->set_visible(true);

  app.run();
  return 0;
}
