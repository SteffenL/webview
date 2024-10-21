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

#ifndef WEBVIEW_DETAIL_UI_WINDOW_BASE_HH
#define WEBVIEW_DETAIL_UI_WINDOW_BASE_HH

#include "../../detail/signal.hh"
#include "widget_base.hh"

#include <memory>
#include <string>

namespace webview {

class window_events {
public:
  detail::signal<void()> ready;
  detail::signal<void()> close_requested;
  detail::signal<void()> destroy;
};

class window_base {
public:
  virtual ~window_base() = default;

  virtual window_events &events() = 0;
  virtual void dispatch(dispatch_fn_t f) = 0;
  virtual void *get_native_handle() const = 0;
  virtual widget_ptr get_widget() = 0;

  virtual void set_title(const std::string &title) = 0;
  virtual void set_visible(bool visible) = 0;
  virtual void close() = 0;
  virtual void destroy() = 0;
};

using window_ptr = std::shared_ptr<window_base>;

} // namespace webview

#endif // WEBVIEW_DETAIL_UI_WINDOW_BASE_HH
