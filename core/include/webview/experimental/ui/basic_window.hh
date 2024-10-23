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

#ifndef WEBVIEW_DETAIL_UI_BASIC_WINDOW_HH
#define WEBVIEW_DETAIL_UI_BASIC_WINDOW_HH

#include "../../detail/signal.hh"
#include "event_loop_base.hh"
#include "widget.hh"
#include "window_options.hh"
#include "window_base.hh"

#include <memory>

#if defined(WEBVIEW_PLATFORM_LINUX)
#include "linux/gtk/window.hh"
#endif

namespace webview {

class basic_window {
public:
  basic_window(const window_options &options = {},
         event_loop_ptr loop)
      : m_event_loop{loop}, m_impl{m_event_loop} {
    m_impl.set_initial_size(options.get_size());
    set_title(options.get_title());
    bind_impl_events();
    m_widget = m_impl->create_widget();
  }

  void set_title(const std::string &title) {
    dispatch([=] { m_impl.set_title(title); });
  }

  void set_visible(bool visible) {
    dispatch([=] { m_impl.set_visible(visible); });
  }

  void close() {
    dispatch([=] { m_impl.close(); });
  }

  void destroy() {
    dispatch([=] { m_impl.destroy(); });
  }

  void dispatch(dispatch_fn_t f) {
    if (!is_valid()) {
      return;
    }
    m_event_loop->dispatch([=] {
      if (is_valid()) {
        f();
      }
    });
  }

  window_events &events() { return m_events; }
  widget& get_widget() { return m_widget; }
  void *get_native_handle() const { return m_impl.get_native_handle(); }

private:
  bool is_valid() const { return m_valid; }

  void bind_impl_events() {
    m_impl.events().ready.bind(
        [=] { dispatch([=] { m_events.ready.emit(); }); });
    m_impl.events().close_requested.bind(
        [=] { dispatch([=] { m_events.close_requested.emit(); }); });
    m_impl.events().destroy.bind([=] {
      dispatch([=] {
        m_valid = false;
        m_events.destroy.emit();
      });
    });
  }

  window_events m_events;
  std::shared_ptr<event_loop> m_event_loop;
  widget m_widget;
  detail::window_impl m_impl;
  bool m_valid{true};
};

} // namespace webview

#endif // WEBVIEW_DETAIL_UI_BASIC_WINDOW_HH
