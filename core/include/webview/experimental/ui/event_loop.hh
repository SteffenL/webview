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

#ifndef WEBVIEW_DETAIL_UI_RUN_LOOP_HH
#define WEBVIEW_DETAIL_UI_RUN_LOOP_HH

#include "../../macros.h"

#if defined(WEBVIEW_PLATFORM_LINUX)
#include "linux/gtk_event_loop.hh"
#endif

#include <memory>

namespace webview {

class event_loop {
public:
  static std::shared_ptr<event_loop> get_default() {
    static std::shared_ptr<event_loop> instance;
    if (!instance) {
      instance = std::shared_ptr<event_loop>{new event_loop{}};
    }
    return instance;
  }

  void run() {
    m_stop_run_loop = false;
    while (!m_stop_run_loop) {
      m_impl.iterate(true);
    }
  }

  void iterate(bool block) { m_impl.iterate(block); }

  void stop() {
    dispatch([&] { m_stop_run_loop = true; });
  }

  void dispatch(dispatch_fn_t f) { m_impl.dispatch(f); }

private:
  detail::event_loop_impl m_impl;
  bool m_stop_run_loop{};
};

} // namespace webview

#endif // WEBVIEW_DETAIL_UI_RUN_LOOP_HH
