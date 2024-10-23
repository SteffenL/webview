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

#ifndef WEBVIEW_UI_OPTIONS_HH
#define WEBVIEW_UI_OPTIONS_HH

#include "primitives.h"
#include <string>

namespace webview {

class browser_options {};
class widget_options {};

class window_options {
public:
  static ui_size get_default_ui_size() noexcept {
    static constexpr const ui_size size{480, 320};
    return size;
  }

  const std::string &get_title() const noexcept { return m_title; }

  window_options &set_title(const std::string &title) {
    m_title = title;
    return *this;
  }

  /*window_options &set_visible(bool visible = true) {
      m_visible = visible;
      return *this;
    }*/

  const ui_size &get_size() const noexcept { return m_size; }

  window_options &set_size(const ui_size &size) {
    m_size = size;
    return *this;
  }

  const widget_options &get_widget_options() const noexcept {
    return m_widget_options;
  }

  window_options &set_widget_options(const widget_options &options) {
    m_widget_options = options;
    return *this;
  }

private:
  std::string m_title;
  //bool m_visible;
  ui_size m_size{get_default_ui_size()};
  widget_options m_widget_options;
};

} // namespace webview

#endif // WEBVIEW_UI_OPTIONS_HH
