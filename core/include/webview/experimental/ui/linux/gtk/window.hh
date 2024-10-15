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

#if !defined(WEBVIEW_DETAIL_UI_LINUX_GTK_WINDOW_HH) &&                         \
    defined(WEBVIEW_PLATFORM_LINUX) && defined(WEBVIEW_GTK)
#define WEBVIEW_DETAIL_UI_LINUX_GTK_WINDOW_HH

#include "../../../../detail/platform/linux/gtk/compat.hh"
#include "../../../../detail/signal.hh"
#include "../../../../types.hh"
#include "../../event_loop.hh"
#include "../../primitives.h"
#include "ref.hh"

#include <gtk/gtk.h>

#include <memory>

namespace webview {
namespace detail {

class gtk_window {
public:
  class events_t {
  public:
    signal<void()> ready;
    signal<void()> close_requested;
  };

  gtk_window(std::shared_ptr<event_loop> loop)
      : m_event_loop{loop},
        m_native_window{GTK_WINDOW(gtk_compat::window_new())} {
    m_close_request_conn = gtk_compat::connect_window_close_request(m_native_window.get(), [=] {
      m_event_loop->dispatch([=] { m_events.close_requested.emit(); });
    });
    m_event_loop->dispatch([=] { m_events.ready.emit(); });
  }

  gtk_window(const gtk_window &) = delete;
  gtk_window &operator=(const gtk_window &) = delete;
  gtk_window(gtk_window &&) = delete;
  gtk_window &operator=(gtk_window &&) = delete;
  ~gtk_window() = default;

  void set_initial_size(const ui_size &size) {
    gtk_window_set_default_size(m_native_window.get(),
                                static_cast<int>(size.width),
                                static_cast<int>(size.height));
  }

  void set_title(const std::string &title) {
    gtk_window_set_title(m_native_window.get(), title.c_str());
  }

  void set_visible(bool visible) {
    gtk_compat::widget_set_visible(GTK_WIDGET(m_native_window.get()), visible);
  }

  void close() { gtk_window_close(m_native_window.get()); }

  events_t &events() { return m_events; }
  void *get_native_handle() const { return m_native_window.get(); }

  widget &get_widget() { return *m_widget; }

  void add_widget() {
    m_widget = std::unique_ptr<widget>{new widget{m_event_loop}};
    gtk_compat::window_set_child(
        m_native_window.get(),
        static_cast<GtkWidget *>(m_widget->get_native_handle()));
    gtk_compat::widget_set_visible(
        static_cast<GtkWidget *>(m_widget->get_native_handle()), true);
  }

private:
  events_t m_events;
  std::shared_ptr<event_loop> m_event_loop;
  gtk_ref<GtkWindow> m_native_window;
  std::unique_ptr<widget> m_widget;
  gtk_compat::connection m_close_request_conn;
};

using window_impl = gtk_window;

} // namespace detail
} // namespace webview

#endif // WEBVIEW_DETAIL_UI_LINUX_GTK_WINDOW_HH
