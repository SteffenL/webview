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

#ifndef WEBVIEW_UI_APPLICATION_HH
#define WEBVIEW_UI_APPLICATION_HH

#include "backends.hh"
#include "event_loop.hh"
#include "options.hh"

namespace webview {

class application : public detail::application_impl {
public:
  application(event_loop_ptr loop = event_loop::get_default())
      : detail::application_impl{loop}, m_event_loop{loop} {}

  static application_ptr get_default() {
    static application_ptr instance;
    if (!instance) {
      instance = application_ptr{new application{}};
    }
    return instance;
  }

  void run() override { m_event_loop->run(); }
  void terminate() override { m_event_loop->stop(); }
  void dispatch(dispatch_fn_t f) override { m_event_loop->dispatch(f); }
  application_events &events() override { return m_events; }

private:
  application_events m_events;
  event_loop_ptr m_event_loop;
};

} // namespace webview

#endif // WEBVIEW_UI_APPLICATION_HH
