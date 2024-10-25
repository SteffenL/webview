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

#ifndef WEBVIEW_DETAIL_LINUX_WEBKITGTK_USER_SCRIPT_HH
#define WEBVIEW_DETAIL_LINUX_WEBKITGTK_USER_SCRIPT_HH

#include "../../../../macros.h"

#if defined(WEBVIEW_PLATFORM_LINUX) && defined(WEBVIEW_GTK)

#include "../../user_script_base.hh"

#include <gtk/gtk.h>

#if GTK_MAJOR_VERSION >= 4
#include <webkit/webkit.h>
#elif GTK_MAJOR_VERSION >= 3
#include <webkit2/webkit2.h>
#endif

#include <string>

namespace webview {
namespace detail {

class webkitgtk_user_script : public user_script_base {
public:
  explicit webkitgtk_user_script(const std::string &code,
                                 WebKitUserScript *native_script)
      : m_code{code}, m_native_script{webkit_user_script_ref(native_script)} {}

  webkitgtk_user_script(const webkitgtk_user_script &) = delete;
  webkitgtk_user_script &operator=(const webkitgtk_user_script &) = delete;

  webkitgtk_user_script(webkitgtk_user_script &&other) noexcept {
    *this = std::move(other);
  }

  webkitgtk_user_script &operator=(webkitgtk_user_script &&other) noexcept {
    if (this != &other) {
      m_code = std::move(other.m_code);

      m_native_script = other.m_native_script;
      other.m_native_script = nullptr;
    }
    return *this;
  }

  virtual ~webkitgtk_user_script() {
    if (m_native_script) {
      webkit_user_script_unref(m_native_script);
    }
  }

  void *get_native_handle() const override { return m_native_script; }
  const std::string &get_code() const override { return m_code; }

private:
  std::string m_code;
  WebKitUserScript *m_native_script{};
};

} // namespace detail
} // namespace webview

#endif
#endif // WEBVIEW_DETAIL_LINUX_WEBKITGTK_USER_SCRIPT_HH
