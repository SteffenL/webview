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

#ifndef WEBVIEW_WIDGET_HH
#define WEBVIEW_WIDGET_HH

#include "backends.hh"
#include "browser.hh"
#include "event_loop.hh"
#include "options.hh"

namespace webview {

class widget : public detail::widget_impl {
public:
  widget(const widget_options &options = {},
         event_loop_ptr loop = event_loop::get_default())
      : m_event_loop{loop},
        m_browser{new class browser{options.get_browser_options(), loop}} {
    embed(m_browser->get_native_embeddable());
  }

  widget_events &events() override { return m_events; }
  void dispatch(dispatch_fn_t f) override { m_event_loop->dispatch(f); }
  browser_ptr browser() override { return m_browser; }

private:
  widget_events m_events;
  event_loop_ptr m_event_loop;
  browser_ptr m_browser;
};

} // namespace webview

#endif // WEBVIEW_WIDGET_HH
