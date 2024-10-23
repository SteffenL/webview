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

#ifndef WEBVIEW_UI_HH
#define WEBVIEW_UI_HH

#include "ui/backends.hh"
#include "ui/options.hh"

namespace webview {

class event_loop : public detail::event_loop_impl {
public:
  static event_loop_ptr get_default() {
    static event_loop_ptr instance;
    if (!instance) {
      instance = event_loop_ptr{new event_loop{}};
    }
    return instance;
  }
};

class widget : public detail::widget_impl {
public:
  widget(const widget_options & /*options*/ = {},
         event_loop_ptr loop = event_loop::get_default())
      : detail::widget_impl{loop} {}

  static widget_ptr get_default() {
    static widget_ptr instance;
    if (!instance) {
      instance = widget_ptr{new widget{}};
    }
    return instance;
  }
};

class window : public detail::window_impl {
public:
  window(const window_options &options = {},
         event_loop_ptr loop = event_loop::get_default())
      : detail::window_impl{loop} {
    set_initial_size(options.get_size());
    set_title(options.get_title());
    set_widget(widget_ptr{new widget{options.get_widget_options(), loop}});
  }

  static window_ptr get_default() {
    static window_ptr instance;
    if (!instance) {
      instance = window_ptr{new window{}};
    }
    return instance;
  }
};

class application : public detail::application_impl {
public:
  application(event_loop_ptr loop = event_loop::get_default())
      : detail::application_impl{loop} {}

  static application_ptr get_default() {
    static application_ptr instance;
    if (!instance) {
      instance = application_ptr{new application{}};
    }
    return instance;
  }
};

} // namespace webview

#endif // WEBVIEW_UI_HH
