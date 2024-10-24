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

#ifndef WEBVIEW_UI_WINDOW_HH
#define WEBVIEW_UI_WINDOW_HH

#include "backends.hh"
#include "event_loop.hh"
#include "options.hh"

namespace webview {

class window : public detail::window_impl<window> {
public:
  window(const window_options &options = {},
         event_loop_ptr loop = event_loop::get_default())
      : m_widget{new class widget{options.get_widget_options(), loop}} {
    set_initial_size(options.get_size());
    set_title(options.get_title());
    set_widget(m_widget);
  }

  static window_ptr get_default() {
    static window_ptr instance;
    if (!instance) {
      instance = window_ptr{new window{}};
    }
    return instance;
  }

  void dispatch(dispatch_fn_t f) override { m_event_loop->dispatch(f); }

  widget_ptr widget() override { return m_widget; }
  browser_ptr browser() override { return m_widget->browser(); }
  detail::window_events<window> &events() override { return m_events; }

private:
  event_loop_ptr m_event_loop;
  widget_ptr m_widget;
  detail::window_events<window> m_events;
};

} // namespace webview

#endif // WEBVIEW_UI_WINDOW_HH
