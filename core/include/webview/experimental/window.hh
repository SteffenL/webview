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

#ifndef WEBVIEW_WINDOW_HH
#define WEBVIEW_WINDOW_HH

#include "backends.hh"
#include "event_loop.hh"
#include "options.hh"
#include "widget.hh"

namespace webview {

class window : public detail::window_impl {
public:
  window(const window_options &options = {},
         event_loop_ptr loop = event_loop::get_default())
      : m_event_loop{loop},
        m_widget{new class widget{options.get_widget_options(), loop}} {
    set_initial_size(options.get_size());
    set_title(options.get_title());
    embed(m_widget->get_native_handle());
    bind_default_event_handlers();
    bind_native_events();
  }

  void dispatch(dispatch_fn_t f) override { m_event_loop->dispatch(f); }
  widget_ptr widget() override { return m_widget; }
  browser_ptr browser() override { return m_widget->browser(); }
  window_events &events() override { return m_events; }

private:
  void bind_default_event_handlers() noexcept {
    events().close_requested.bind(
        [](iwindow *sender) {
          sender->destroy();
          return true;
        },
        detail::get_internal_signal_priority());
  }

  event_loop_ptr m_event_loop;
  widget_ptr m_widget;
  window_events m_events;
};

} // namespace webview

#endif // WEBVIEW_WINDOW_HH
