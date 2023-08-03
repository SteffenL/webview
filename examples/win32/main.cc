#include "webview.h"

#include <windows.h>

#include <memory>
#include <string>

constexpr const auto html =
    R"html(<button id="increment">Tap me</button>
<script>
  const [incrementElement] = document.querySelectorAll("#increment");
  document.addEventListener("DOMContentLoaded", () => {
    incrementElement.addEventListener("click", () => {
      window.increment();
    });
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
      // Create fonts
      NONCLIENTMETRICSW ncm{};
      ncm.cbSize = sizeof(ncm);
      SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
      auto default_font{CreateFontIndirectW(&ncm.lfMessageFont)};
      ncm.lfMessageFont.lfHeight = -72;
      auto counter_font{CreateFontIndirectW(&ncm.lfMessageFont)};

      // Crate the location edit control
      m_location_edit = CreateWindowExW(WS_EX_CLIENTEDGE, L"Edit", nullptr,
                                        WS_CHILD | WS_VISIBLE, 0, 0, 0, 0,
                                        m_hwnd, nullptr, m_instance, nullptr);
      SendMessageW(m_location_edit, WM_SETFONT,
                   reinterpret_cast<WPARAM>(default_font), 0);
      SetWindowTextW(m_location_edit, L"https://github.com/webview/webview");

      // Create the go button control
      m_go_button =
          CreateWindowExW(0, L"Button", nullptr, WS_CHILD | WS_VISIBLE, 0, 0, 0,
                          0, m_hwnd, nullptr, m_instance, nullptr);
      SendMessageW(m_go_button, WM_SETFONT,
                   reinterpret_cast<WPARAM>(default_font), 0);
      SetWindowTextW(m_go_button, L"Go");

      // Create the counter static control
      m_counter_static =
          CreateWindowExW(0, L"Static", nullptr,
                          WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, 0,
                          0, 0, 0, m_hwnd, nullptr, m_instance, nullptr);
      SendMessageW(m_counter_static, WM_SETFONT,
                   reinterpret_cast<WPARAM>(counter_font), 0);
      SetWindowTextW(m_counter_static, L"0");

      // Create webview instance
      m_webview = std::unique_ptr<webview::webview>{
          new webview::webview{false, m_hwnd}};

      m_webview->bind("increment",
                      [this](const std::string & /*req*/) -> std::string {
                        auto text{std::to_string(++m_counter)};
                        SetWindowTextA(m_counter_static, text.c_str());
                        return "";
                      });

      m_webview->set_html(html);
      break;
    }

    case WM_SIZE: {
      // Update the UI layout
      static constexpr const int top_neight{20};
      static constexpr const int button_width{40};
      RECT main_rect{};
      GetClientRect(m_hwnd, &main_rect);
      auto main_width{main_rect.right - main_rect.left};
      auto main_half_width{main_width / 2};
      auto main_height{main_rect.bottom - main_rect.top};
      auto location_edit_width = main_width - button_width;
      MoveWindow(m_location_edit, main_rect.left, main_rect.top,
                 location_edit_width, top_neight, TRUE);
      MoveWindow(m_go_button, main_rect.left + location_edit_width,
                 main_rect.top, button_width, top_neight, TRUE);
      MoveWindow(static_cast<HWND>(m_webview->widget()), main_rect.left,
                 main_rect.top + top_neight, main_width / 2,
                 main_height - top_neight, TRUE);
      MoveWindow(m_counter_static, main_rect.left + main_half_width,
                 main_rect.top + top_neight, main_width / 2,
                 main_height - top_neight, TRUE);
      InvalidateRect(m_counter_static, nullptr, TRUE);
      break;
    }

    case WM_CLOSE:
      DestroyWindow(m_hwnd);
      break;

    case WM_DESTROY:
      PostQuitMessage(0);
      break;

    case WM_COMMAND:
      if (reinterpret_cast<HWND>(lp) == m_go_button) {
        // Update the counter static control when the go button is pressed
        auto length{GetWindowTextLengthW(m_location_edit)};
        std::wstring url(length + 1, 0);
        GetWindowTextW(m_location_edit, url.data(), url.size());
        url.resize(length);
        // webview internals are used here for simplicity - you should use your
        // own solution.
        m_webview->navigate(webview::detail::narrow_string(url));
      }
      break;

    default:
      return DefWindowProcW(m_hwnd, msg, wp, lp);
    }

    return 0;
  }

  HINSTANCE m_instance{};
  std::unique_ptr<webview::webview> m_webview;
  int m_counter{};
  HWND m_hwnd{};
  HWND m_location_edit{};
  HWND m_go_button{};
  HWND m_counter_static{};
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nShowCmd) {
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
