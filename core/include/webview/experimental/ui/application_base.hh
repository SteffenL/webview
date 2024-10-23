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

#ifndef WEBVIEW_DETAIL_UI_APPLICATION_BASE_HH
#define WEBVIEW_DETAIL_UI_APPLICATION_BASE_HH

#include "../../detail/signal.hh"
#include "event_loop_base.hh"

#include <memory>

namespace webview {

class application_events {
public:
  detail::signal<void()> ready;
};

class application_base {
public:
  explicit application_base(event_loop_ptr event_loop)
      : m_event_loop{event_loop} {}
  virtual ~application_base() = default;

  void run() { run_impl(); }
  void terminate() { terminate_impl(); }
  void dispatch(dispatch_fn_t f) { dispatch_impl(f); }
  application_events &events() { return events_impl(); }

protected:
  virtual void run_impl() { m_event_loop->run(); }
  virtual void terminate_impl() { m_event_loop->stop(); }
  virtual void dispatch_impl(dispatch_fn_t f) { m_event_loop->dispatch(f); }
  virtual application_events &events_impl() = 0;

private:
  event_loop_ptr m_event_loop;
};

using application_ptr = std::shared_ptr<application_base>;

} // namespace webview

#endif // WEBVIEW_DETAIL_UI_APPLICATION_BASE_HH
