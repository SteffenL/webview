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

#if !defined(WEBVIEW_DETAIL_UI_LINUX_GTK_APPLICATION_HH) &&                    \
    defined(WEBVIEW_PLATFORM_LINUX) && defined(WEBVIEW_GTK)
#define WEBVIEW_DETAIL_UI_LINUX_GTK_APPLICATION_HH

#include "../../../types.hh"
#include "../run_loop.hh"
#include "gtk_ref.hh"

#include <gtk/gtk.h>

namespace webview {
namespace detail {

class gtk_application {
public:
  gtk_application(std::shared_ptr<run_loop> run_loop) : m_run_loop{run_loop} {
    m_native_app = gtk_application_new(nullptr, G_APPLICATION_DEFAULT_FLAGS);

    auto on_activate{+[](GtkApplication *, gpointer user_data) {
      auto *self = static_cast<gtk_application *>(user_data);
      self->on_activate();
    }};

    g_signal_connect(G_OBJECT(m_native_app.get()), "activate",
                     G_CALLBACK(on_activate), this);
  }

  gtk_application(const gtk_application &) = delete;
  gtk_application &operator=(const gtk_application &) = delete;
  gtk_application(gtk_application &&) = delete;
  gtk_application &operator=(gtk_application &&) = delete;
  ~gtk_application() = default;

  void run() { m_run_loop->run(); }
  void terminate() { m_run_loop->stop(); }
  void dispatch(dispatch_fn_t f) { m_run_loop->dispatch(f); }

  bool is_ready() const noexcept { return m_ready; }

private:
  void on_activate() { m_ready = true; }

  bool m_ready{};
  std::shared_ptr<run_loop> m_run_loop;
  gtk_ref<GtkApplication> m_native_app;
};

using application_impl = gtk_application;

} // namespace detail
} // namespace webview

#endif // WEBVIEW_DETAIL_UI_LINUX_GTK_APPLICATION_HH
