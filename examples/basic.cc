#include "webview.h"

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInt, HINSTANCE hPrevInst, LPSTR lpCmdLine,
                   int nCmdShow) {
#else
int main() {
#endif
  webview::webview w(false, nullptr);
  w.set_title("Basic Example");
  w.set_size(480, 320, WEBVIEW_HINT_NONE);
  for (int i = 0; i < 100000; ++i) {
    w.set_html("Hello");
  }
  w.run();
  return 0;
}
