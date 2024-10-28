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

#ifndef WEBVIEW_WINDOW_MANAGER_HH
#define WEBVIEW_WINDOW_MANAGER_HH

#include "detail/window_manager_base.hh"
#include "options.hh"
#include "window.hh"

namespace webview {

class window_manager : public detail::window_manager_base {
public:
  window_manager(event_loop_ptr loop = event_loop::get_default())
      : m_event_loop{loop} {
    bind_default_event_handlers();
  }

  window_manager_events &events() override { return m_events; }

private:
  window_ptr new_window_impl(const window_options &options) override {
    return window_ptr{new window{options, m_event_loop}};
  }

  event_loop_ptr m_event_loop;
  window_manager_events m_events;
};

} // namespace webview

#endif // WEBVIEW_WINDOW_MANAGER_HH
