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

#ifndef WEBVIEW_DETAIL_UI_WINDOW_HH
#define WEBVIEW_DETAIL_UI_WINDOW_HH

#include "../../detail/signal.hh"
#include "event_loop.hh"
#include "widget.hh"
#include "window_options.hh"

#include <memory>

#if defined(WEBVIEW_PLATFORM_LINUX)
#include "linux/gtk/window.hh"
#endif

namespace webview {

class window {
public:
  class events_t {
  public:
    detail::signal<void()> ready;
    detail::signal<void()> close_requested;
  };

  window(const window_options &options = {},
         std::shared_ptr<event_loop> loop = event_loop::get_default())
      : m_event_loop{loop}, m_impl{m_event_loop} {
    m_impl.set_initial_size(options.get_size());
    set_title(options.get_title());
    m_impl.add_widget();
    m_impl.events().ready.bind([=] { m_events.ready.emit(); });
    m_impl.events().close_requested.bind(
        [=] { m_events.close_requested.emit(); });
  }

  void set_title(const std::string &title) {
    m_event_loop->dispatch([=] { m_impl.set_title(title); });
  }

  void set_visible(bool visible) {
    m_event_loop->dispatch([=] { m_impl.set_visible(visible); });
  }

  void close() {
    m_event_loop->dispatch([=] { m_impl.close(); });
  }

  void dispatch(dispatch_fn_t f) { m_event_loop->dispatch(f); }
  events_t &events() { return m_events; }
  widget &get_widget() { return m_impl.get_widget(); }
  void *get_native_handle() const { return m_impl.get_native_handle(); }

private:
  events_t m_events;
  std::shared_ptr<event_loop> m_event_loop;
  detail::window_impl m_impl;
};

} // namespace webview

#endif // WEBVIEW_DETAIL_UI_WINDOW_HH
