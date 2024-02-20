#include "webview.h"

#include <windows.h>

#include <memory>
#include <string>

constexpr const auto html1 =
    R"html(<button id="ping">Ping</button>
<p id="received-container" style="display: none">Received pong!</p>
<script type="module">
  const elements = {
    pingButton: document.getElementById("ping"),
    receivedContainer: document.getElementById("received-container")
  }
  window.pong = () => elements.receivedContainer.style.display = "block";
  elements.pingButton.addEventListener("click", () => {
    elements.receivedContainer.style.display = "none";
    window.ping();
  });
</script>)html";

constexpr const auto html2 =
    R"html(<p id="received-container" style="display: none">Received ping!
  <button id="pong">Pong</button></p>
<script type="module">
  const elements = {
    pongButton: document.getElementById("pong"),
    receivedContainer: document.getElementById("received-container")
  }
  window.ping = () => elements.receivedContainer.style.display = "block";
  elements.pongButton.addEventListener("click", () => {
    elements.receivedContainer.style.display = "none";
    window.pong();
  });
</script>)html";

class MainWindow {
public:
  MainWindow(const wchar_t *class_name, const wchar_t *title, int x, int y,
             int width, int height)
      : m_instance{GetModuleHandle(nullptr)} {
    // Create top-level window
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = wndproc_wrapper;
    wc.hInstance = m_instance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = class_name;
    RegisterClassExW(&wc);
    CreateWindowExW(0, class_name, title, WS_OVERLAPPEDWINDOW, x, y, width,
                    height, nullptr, nullptr, m_instance, this);
  }

  void show() {
    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);
  }

private:
  static LRESULT wndproc_wrapper(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    MainWindow *self{};
    if (msg == WM_NCCREATE) {
      auto *lpcs{reinterpret_cast<LPCREATESTRUCT>(lp)};
      self = static_cast<MainWindow *>(lpcs->lpCreateParams);
      self->m_hwnd = hwnd;
      SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    } else {
      self = reinterpret_cast<MainWindow *>(
          GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (!self) {
      return DefWindowProcW(hwnd, msg, wp, lp);
    }

    return self->wndproc(msg, wp, lp);
  }

  LRESULT wndproc(UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE: {
      // Create webview instances
      m_webview1 = std::make_unique<webview::webview>(false, m_hwnd);
      m_webview2 = std::make_unique<webview::webview>(false, m_hwnd);

      m_webview1->bind("ping",
                       [this](const std::string &req) -> std::string {
                         // Forward to another webview instance.
                         m_webview2->eval("window.ping(" + req + ")");
                         return "";
                       });

      m_webview2->bind("pong",
                       [this](const std::string &req) -> std::string {
                         // Respond to another webview instance.
                         m_webview1->eval("window.pong(" + req + ")");
                         return "";
                       });

      m_webview1->set_html(html1);
      m_webview2->set_html(html2);
      break;
    }

    case WM_SIZE: {
      // Update the UI layout
      RECT main_rect{};
      GetClientRect(m_hwnd, &main_rect);
      auto main_width{main_rect.right - main_rect.left};
      auto main_half_width{main_width / 2};
      auto main_height{main_rect.bottom - main_rect.top};
      MoveWindow(static_cast<HWND>(m_webview1->widget()), main_rect.left,
                 main_rect.top, main_width / 2, main_height, TRUE);
      MoveWindow(static_cast<HWND>(m_webview2->widget()),
                 main_rect.left + main_half_width, main_rect.top,
                 main_width / 2, main_height, TRUE);
      break;
    }

    case WM_CLOSE:
      DestroyWindow(m_hwnd);
      break;

    case WM_DESTROY:
      PostQuitMessage(0);
      break;

    default:
      return DefWindowProcW(m_hwnd, msg, wp, lp);
    }

    return 0;
  }

  HINSTANCE m_instance{};
  std::unique_ptr<webview::webview> m_webview1;
  std::unique_ptr<webview::webview> m_webview2;
  HWND m_hwnd{};
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nShowCmd) {
  CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

  // Create the main window
  MainWindow window{
      L"app_window", L"Win32 Example", CW_USEDEFAULT, CW_USEDEFAULT, 480, 320};
  window.show();

  // The message loop
  MSG msg;
  while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }

  return 0;
}