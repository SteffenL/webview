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

#ifndef WEBVIEW_UI_DETAIL_RUN_LOOP_BASE_HH
#define WEBVIEW_UI_DETAIL_RUN_LOOP_BASE_HH

#include "../../../types.hh"

#include <memory>

namespace webview {
namespace detail {

class event_loop_base {
public:
  virtual ~event_loop_base() = default;

  void run() {
    m_stop_run_loop = false;
    while (!m_stop_run_loop) {
      iterate(true);
    }
  }

  virtual void iterate(bool block) = 0;

  void stop() {
    dispatch([&] { m_stop_run_loop = true; });
  }

  virtual void dispatch(dispatch_fn_t f) = 0;

private:
  bool m_stop_run_loop{};
};

} // namespace detail

using event_loop_ptr = std::shared_ptr<detail::event_loop_base>;

} // namespace webview

#endif // WEBVIEW_UI_DETAIL_RUN_LOOP_BASE_HH
