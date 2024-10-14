#include "webview/experimental/ui.hh"

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE /*hInst*/, HINSTANCE /*hPrevInst*/,
                   LPSTR /*lpCmdLine*/, int /*nCmdShow*/) {
#else
int main() {
#endif
  webview::application app;
  webview::window window{webview::window_options{}.set_title("Hello")};
  window.set_visible(true);
  app.run();
  return 0;
}
