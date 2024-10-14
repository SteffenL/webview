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

#if !defined(WEBVIEW_DETAIL_UI_LINUX_GTK_REF_HH) &&                            \
    defined(WEBVIEW_PLATFORM_LINUX) && defined(WEBVIEW_GTK)
#define WEBVIEW_DETAIL_UI_LINUX_GTK_REF_HH

#include <utility>

#include <gtk/gtk.h>

namespace webview {
namespace detail {

template <typename T> class gtk_ref {
public:
  gtk_ref() = default;
  gtk_ref(T *ptr) : m_ptr{ptr} { ref(); }

  gtk_ref(const gtk_ref &other) { *this = other; }

  gtk_ref &operator=(const gtk_ref &other) {
    if (this != &other) {
      m_ptr = other.m_ptr;
      ref();
    }
    return *this;
  }

  gtk_ref(gtk_ref &&other) noexcept { *this = std::move(other); }

  gtk_ref &operator=(gtk_ref &&other) noexcept {
    m_ptr = other.m_ptr;
    other.m_ptr = nullptr;
    return *this;
  }

  ~gtk_ref() {
    if (m_ptr) {
      unref();
    }
  }

  T *get() const noexcept { return m_ptr; }

private:
  void ref() { g_object_ref(m_ptr); }
  void unref() { g_object_unref(m_ptr); }

  T *m_ptr{};
};

} // namespace detail
} // namespace webview

#endif // WEBVIEW_DETAIL_UI_LINUX_GTK_REF_HH
