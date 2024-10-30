#include "resources/next_api/html.hh"
#include "webview/experimental/webview.hh"

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

#include <iostream>

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
#else
int main() {
#endif
  webview::application app;
  webview::window_manager wm;
  webview::task_executor bridge_tasks;
  webview::task_executor scheme_tasks{std::thread::hardware_concurrency()};

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
  main_bridge->bind("runTasks", [&bridge_tasks](webview::binding_arg &arg) {
    bridge_tasks.put(
        [](webview::binding_promise promise) {
          static std::atomic_int i{};
          std::this_thread::sleep_for(std::chrono::seconds{1});
          promise.resolve(std::to_string(++i));
        },
        arg.get_promise());
  });
  main->browser()->set_html(make_html(main_window_html));
  main->browser()->uri_schemes()->bind(
      "app", [&scheme_tasks](const webview::http::request &request,
                             webview::http::response_promise response) {
        std::string headers;
        for (const auto &header : request.get_headers()) {
          headers += "header: ";
          headers += header.first;
          headers += ": ";
          headers += header.second;
          headers += '\n';
        }
        std::cout << "req: " << request.get_method() << ' '
                  << request.get_path() << '\n'
                  << headers << '\n';
        //scheme_tasks.put(
        //    [](webview::http::response_promise response) {
        response.resolve(
            webview::http::response{{200, "OK"}}.set_content_type("text/html"));
        //     },
        //     response);
      });
  main->set_visible(true);

  app.run();
  return 0;
}
