#include "webview.h"

#include <commctrl.h>
#include <errhandlingapi.h>
#include <windows.h>

#include <memory>
#include <string>
#include <winuser.h>

struct app_context_t {
  HINSTANCE hInstance{};
  std::unique_ptr<webview::webview> w;
  int counter{};
  HWND window{};
  HWND location_entry{};
  HWND go_button{};
  HWND counter_text{};
};

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

enum control_id_t { ID_LOCATION_EDIT, ID_GO_BUTTON };

LRESULT main_wndproc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  app_context_t *app_context{};
  if (uMsg == WM_NCCREATE) {
    auto *lpcs{reinterpret_cast<LPCREATESTRUCT>(lParam)};
    app_context = static_cast<app_context_t *>(lpcs->lpCreateParams);
    app_context->window = hWnd;
    SetWindowLongPtrW(hWnd, GWLP_USERDATA,
                      reinterpret_cast<LONG_PTR>(app_context));
  } else {
    app_context = reinterpret_cast<app_context_t *>(
        GetWindowLongPtrW(hWnd, GWLP_USERDATA));
  }

  if (!app_context) {
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
  }

  switch (uMsg) {
  case WM_CREATE: {
    NONCLIENTMETRICSW ncm{};
    ncm.cbSize = sizeof(ncm);
    SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
    auto default_font{CreateFontIndirectW(&ncm.lfMessageFont)};
    ncm.lfMessageFont.lfHeight = -72;
    auto counter_font{CreateFontIndirectW(&ncm.lfMessageFont)};

    // TODO: Ctrl+A doesn't work in edit field
    app_context->location_entry = CreateWindowExW(
        WS_EX_CLIENTEDGE, L"Edit", nullptr, WS_CHILD | WS_VISIBLE, 0, 0, 0, 0,
        hWnd, nullptr, app_context->hInstance, nullptr);
    SendMessageW(app_context->location_entry, WM_SETFONT,
                 reinterpret_cast<WPARAM>(default_font), 0);
    SetWindowTextW(app_context->location_entry,
                   L"https://github.com/webview/webview");

    app_context->go_button =
        CreateWindowExW(0, L"Button", nullptr, WS_CHILD | WS_VISIBLE, 0, 0, 0,
                        0, hWnd, nullptr, app_context->hInstance, nullptr);
    SendMessageW(app_context->go_button, WM_SETFONT,
                 reinterpret_cast<WPARAM>(default_font), 0);
    SetWindowTextW(app_context->go_button, L"Go");

    app_context->counter_text = CreateWindowExW(
        0, L"Static", nullptr,
        WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, 0, 0, 0, 0, hWnd,
        nullptr, app_context->hInstance, nullptr);
    SendMessageW(app_context->counter_text, WM_SETFONT,
                 reinterpret_cast<WPARAM>(counter_font), 0);
    SetWindowTextW(app_context->counter_text, L"0");

    app_context->w =
        std::unique_ptr<webview::webview>{new webview::webview{false, hWnd}};

    app_context->w->bind(
        "increment", [app_context](const std::string & /*req*/) -> std::string {
          auto text{std::to_string(++app_context->counter)};
          SetWindowTextA(app_context->counter_text, text.c_str());
          return "";
        });

    app_context->w->set_html(html);
    break;
  }

  case WM_SIZE: {
    static constexpr const int top_neight{20};
    static constexpr const int button_width{40};
    RECT main_rect{};
    GetClientRect(hWnd, &main_rect);
    auto main_width{main_rect.right - main_rect.left};
    auto main_half_width{main_width / 2};
    auto main_height{main_rect.bottom - main_rect.top};
    auto location_entry_width = main_width - button_width;
    MoveWindow(app_context->location_entry, main_rect.left, main_rect.top,
               location_entry_width, top_neight, TRUE);
    MoveWindow(app_context->go_button, main_rect.left + location_entry_width,
               main_rect.top, button_width, top_neight, TRUE);
    MoveWindow(static_cast<HWND>(app_context->w->widget()), main_rect.left,
               main_rect.top + top_neight, main_width / 2,
               main_height - top_neight, TRUE);
    MoveWindow(app_context->counter_text, main_rect.left + main_half_width,
               main_rect.top + top_neight, main_width / 2,
               main_height - top_neight, TRUE);
    InvalidateRect(app_context->counter_text, nullptr, TRUE);
    break;
  }

  case WM_CLOSE:
    DestroyWindow(hWnd);
    break;

  case WM_DESTROY:
    PostQuitMessage(0);
    break;

  case WM_COMMAND:
    if (reinterpret_cast<HWND>(lParam) == app_context->go_button) {
      auto length{GetWindowTextLengthW(app_context->location_entry)};
      std::wstring url(length + 1, 0);
      GetWindowTextW(app_context->location_entry, url.data(), url.size());
      url.resize(length);
      // webview internals are used here for simplicity - you should use your
      // own solution.
      app_context->w->navigate(webview::detail::narrow_string(url));
    }
    break;

  default:
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
  }

  return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nShowCmd) {
  INITCOMMONCONTROLSEX icc{};
  icc.dwSize = sizeof(icc);
  icc.dwICC = ICC_WIN95_CLASSES | ICC_STANDARD_CLASSES;
  InitCommonControlsEx(&icc);

  app_context_t app_context{};
  app_context.hInstance = hInstance;

  WNDCLASSEXW wc{};
  wc.cbSize = sizeof(WNDCLASSEX);
  wc.lpfnWndProc = main_wndproc;
  wc.hInstance = hInstance;
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
  wc.lpszClassName = L"app_window";
  RegisterClassExW(&wc);
  auto window{CreateWindowExW(
      0, L"app_window", L"Win32 Example", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
      CW_USEDEFAULT, 640, 480, nullptr, nullptr, hInstance, &app_context)};

  ShowWindow(window, SW_SHOW);
  UpdateWindow(window);

  MSG msg;
  while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }

  return 0;
}
