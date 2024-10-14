#include "webview/experimental/ui.hh"

#include <iostream>

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE /*hInst*/, HINSTANCE /*hPrevInst*/,
                   LPSTR /*lpCmdLine*/, int /*nCmdShow*/) {
#else
int main() {
#endif
  webview::application app;
  webview::window window;
  window.set_title("Hello");
  window.set_visible();
  app.run();
  return 0;
}
