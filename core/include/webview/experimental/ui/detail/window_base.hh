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

#ifndef WEBVIEW_UI_DETAIL_WINDOW_BASE_HH
#define WEBVIEW_UI_DETAIL_WINDOW_BASE_HH

#include "../../../detail/signal.hh"
#include "widget_base.hh"

#include <memory>
#include <string>

namespace webview {
namespace detail {

class window_events {
public:
  signal<void()> ready;
  signal<void()> close_requested;
  signal<void()> destroy;
};

class window_base {
public:
  virtual ~window_base() = default;

  window_events &events() { return events_impl(); }
  void dispatch(dispatch_fn_t f) { dispatch_impl(f); }
  void *get_native_handle() const { return get_native_handle_impl(); }

  virtual void set_title(const std::string &title) { set_title_impl(title); }
  virtual void set_visible(bool visible) { set_visible_impl(visible); }
  virtual void close() { close_impl(); }
  virtual void destroy() { destroy_impl(); }

protected:
  void initialize() { set_default_event_handlers(); }

  virtual void set_widget(widget_ptr widget) = 0;

  virtual window_events &events_impl() = 0;
  virtual void dispatch_impl(dispatch_fn_t f) = 0;
  virtual void *get_native_handle_impl() const = 0;

  virtual void set_title_impl(const std::string &title) = 0;
  virtual void set_visible_impl(bool visible) = 0;
  virtual void close_impl() = 0;
  virtual void destroy_impl() = 0;

private:
  void set_default_event_handlers() {
    events().close_requested.bind([=] {
      destroy();
      return true;
    });
  }
};

} // namespace detail

using window_ptr = std::shared_ptr<detail::window_base>;

} // namespace webview

#endif // WEBVIEW_UI_DETAIL_WINDOW_BASE_HH
