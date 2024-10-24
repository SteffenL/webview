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

class ievent_loop {
public:
  virtual ~ievent_loop() = default;

  virtual void run() = 0;
  virtual void iterate(bool block) = 0;
  virtual void stop() = 0;
  virtual void dispatch(dispatch_fn_t f) = 0;
};

using event_loop_ptr = std::shared_ptr<ievent_loop>;

namespace detail {

class event_loop_base : public ievent_loop {
public:
  virtual ~event_loop_base() = default;

  void run() override {
    m_stop_run_loop = false;
    while (!m_stop_run_loop) {
      iterate(true);
    }
  }

  void stop() override {
    dispatch([&] { m_stop_run_loop = true; });
  }

private:
  bool m_stop_run_loop{};
};

} // namespace detail
} // namespace webview

#endif // WEBVIEW_UI_DETAIL_RUN_LOOP_BASE_HH
