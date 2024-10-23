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

#ifndef WEBVIEW_UI_DETAIL_WIDGET_BASE_HH
#define WEBVIEW_UI_DETAIL_WIDGET_BASE_HH

#include "../../../detail/signal.hh"
#include "../../../types.hh"

#include <memory>
#include <string>

namespace webview {
namespace detail {

class widget_events {
public:
  signal<void()> ready;
};

class widget_base {
public:
  virtual ~widget_base() = default;

  widget_events &events() { return events_impl(); }
  void dispatch(dispatch_fn_t f) { dispatch_impl(f); }
  void *get_native_handle() const { return get_native_handle_impl(); }

  void navigate(const std::string &url) { navigate_impl(url); }
  void set_html(const std::string &html) { set_html_impl(html); }

protected:
  virtual widget_events &events_impl() = 0;
  virtual void dispatch_impl(dispatch_fn_t f) = 0;
  virtual void *get_native_handle_impl() const = 0;

  virtual void navigate_impl(const std::string &url) = 0;
  virtual void set_html_impl(const std::string &html) = 0;
};

} // namespace detail

using widget_ptr = std::shared_ptr<detail::widget_base>;

} // namespace webview

#endif // WEBVIEW_UI_DETAIL_WIDGET_BASE_HH
