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

#include "../../macros.h"
#include "run_loop.hh"

#include <memory>

#if defined(WEBVIEW_PLATFORM_LINUX)
#include "linux/gtk_window.hh"
#endif

namespace webview {

class window {
public:
  window() : m_run_loop{new run_loop{}} {
    m_impl = std::unique_ptr<detail::window_impl>{
        new detail::window_impl{m_run_loop}};
    m_impl->events().ready.bind([&] { m_ready = true; });
    while (!m_ready) {
      m_run_loop->iterate(true);
    }
  }

  void run() { m_run_loop->run(); }
  void terminate() { m_run_loop->stop(); }
  void dispatch(dispatch_fn_t f) { m_run_loop->dispatch(f); }

private:
  std::shared_ptr<run_loop> m_run_loop;
  std::unique_ptr<detail::window_impl> m_impl;
  bool m_ready{};
};

} // namespace webview

#endif // WEBVIEW_DETAIL_UI_WINDOW_HH
