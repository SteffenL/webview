/*
 * MIT License
 *
 * Copyright (c) 2017 Serge Zaitsev
 * Copyright (c) 2022 Steffen André Langnes
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef WEBVIEW_DETAIL_WINDOW_MANAGER_BASE_HH
#define WEBVIEW_DETAIL_WINDOW_MANAGER_BASE_HH

#include "../../detail/signal.hh"
#include "../options.hh"
#include "application_base.hh"
#include "window_base.hh"

#include <list>
#include <memory>
#include <unordered_map>

namespace webview {

class iwindow_manager;

class window_manager_events {
public:
  detail::signal<bool(iwindow_manager *sender)> last_window_destroyed;
};

class iwindow_manager {
public:
  virtual ~iwindow_manager() = default;
  virtual window_manager_events &events() = 0;
  virtual window_ptr new_window(const window_options &options) = 0;
};

using window_manager_ptr = std::shared_ptr<iwindow_manager>;

namespace detail {

class window_manager_base : public iwindow_manager {
public:
  virtual ~window_manager_base() = default;

  window_ptr new_window(const window_options &options) override {
    auto window{new_window_impl(options)};
    track_window(window);
    window->events().destroy.bind(
        [this](iwindow *sender) {
          forget_window_by_pointer(sender);
          if (m_windows.empty()) {
            events().last_window_destroyed.emit(this);
          }
          return true;
        },
        detail::get_internal_signal_priority());
    return window;
  }

protected:
  virtual window_ptr new_window_impl(const window_options &options) = 0;

  void bind_default_event_handlers() {
    events().last_window_destroyed.bind(
        [](iwindow_manager * /*sender*/) {
          current_application().terminate();
          return true;
        },
        detail::get_internal_signal_priority());
  }

private:
  void track_window(window_ptr window) {
    m_windows_by_pointer.emplace(window.get(),
                                 m_windows.insert(m_windows.end(), window));
  }

  void forget_window_by_pointer(iwindow *window) {
    auto it{m_windows_by_pointer.find(window)};
    if (it != m_windows_by_pointer.end()) {
      m_windows_by_pointer.erase(window);
      m_windows.erase(it->second);
    }
  }

  std::list<window_ptr> m_windows;
  std::unordered_map<iwindow *, typename decltype(m_windows)::iterator>
      m_windows_by_pointer;
};

} // namespace detail
} // namespace webview

#endif // WEBVIEW_DETAIL_WINDOW_MANAGER_BASE_HH
