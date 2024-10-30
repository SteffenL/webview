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
  webview::task_executor bridge_tasks;
  webview::task_executor scheme_tasks{std::thread::hardware_concurrency()};

  auto main{wm.new_window(
      webview::window_options{}.set_size({480, 320}).set_title("Main Window"))};
  auto main_bridge{main->browser()->bridge()};
  main_bridge->bind("newWindow", [&] {
    auto sub{wm.new_window(webview::window_options{}
                               .set_size({480, 320})
                               .set_title("Sub Window"))};
    sub->browser()->bridge()->bind("closeWindow", [=] { sub->close(); });
    sub->browser()->uri_schemes()->bind(
        "app", [&](const webview::http::request &request,
                   webview::http::response_promise response_promise) {
          scheme_tasks.put(
              [](const webview::http::request &request,
                 webview::http::response_promise promise) {
                if (request.get_method() == "GET") {
                  if (request.get_path() == "/index") {
                    static const std::string html{make_html(sub_window_html)};
                    promise.resolve(
                        webview::http::response{200}.set_content_source(
                            webview::http::content_source::string(
                                html, "text/html")));
                    return;
                  }
                }
                promise.resolve(webview::http::response{404}.set_content_source(
                    webview::http::content_source::string("Not found",
                                                          "text/plain")));
              },
              request, response_promise);
        });
    sub->browser()->navigate("app:///index");
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
  main->browser()->uri_schemes()->bind(
      "app", [&](const webview::http::request &request,
                 webview::http::response_promise response_promise) {
        scheme_tasks.put(
            [](const webview::http::request &request,
               webview::http::response_promise promise) {
              if (request.get_method() == "GET") {
                if (request.get_path() == "/index") {
                  static const std::string html{make_html(main_window_html)};
                  promise.resolve(
                      webview::http::response{200}.set_content_source(
                          webview::http::content_source::string(html,
                                                                "text/html")));
                  return;
                }
              }
              promise.resolve(webview::http::response{404}.set_content_source(
                  webview::http::content_source::string("Not found",
                                                        "text/plain")));
            },
            request, response_promise);
      });
  main->browser()->navigate("app:///index");
  main->set_visible(true);

  app.run();
  return 0;
}
