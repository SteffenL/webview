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

#ifndef WEBVIEW_DETAIL_UI_WIDGET_HH
#define WEBVIEW_DETAIL_UI_WIDGET_HH

#include "event_loop.hh"

#include <memory>

#if defined(WEBVIEW_PLATFORM_LINUX)
#include "linux/gtk/webkitgtk/widget.hh"
#endif

namespace webview {

class widget {
public:
  widget(std::shared_ptr<event_loop> loop = event_loop::get_default())
      : m_event_loop{loop} {
    m_impl = std::unique_ptr<detail::widget_impl>{
        new detail::widget_impl{m_event_loop}};
  }

  void dispatch(dispatch_fn_t f) { m_event_loop->dispatch(f); }

private:
  std::shared_ptr<event_loop> m_event_loop;
  std::unique_ptr<detail::widget_impl> m_impl;
};

} // namespace webview

#endif // WEBVIEW_DETAIL_UI_WINDOW_HH
