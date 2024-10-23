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

#ifndef WEBVIEW_DETAIL_UI_LINUX_GTK_WINDOW_HH
#define WEBVIEW_DETAIL_UI_LINUX_GTK_WINDOW_HH

#include "../../../../../macros.h"

#if defined(WEBVIEW_PLATFORM_LINUX) && defined(WEBVIEW_GTK)

#include "../../../../../detail/platform/linux/gtk/compat.hh"
#include "../../../../../detail/signal.hh"
#include "../../../../../types.hh"
#include "../../../primitives.h"
#include "../../event_loop_base.hh"
#include "../../widget_base.hh"
#include "../../window_base.hh"
#include "gtk_ref.hh"

#include <gtk/gtk.h>

#include <memory>

namespace webview {
namespace detail {

class gtk_window : public window_base {
public:
  explicit gtk_window(event_loop_ptr loop)
      : m_event_loop{loop},
        m_native_window{GTK_WINDOW(gtk_compat::window_new())} {
    initialize();
    m_close_request_conn = gtk_compat::connect_window_close_request(
        m_native_window.get(), [=] { m_events.close_requested.emit(); });
    m_destroy_conn = gtk_compat::connect_widget_destroy(
        GTK_WIDGET(m_native_window.get()), [=] { m_events.destroy.emit(); });
    m_events.ready.emit();
  }

  gtk_window(const gtk_window &) = delete;
  gtk_window &operator=(const gtk_window &) = delete;
  gtk_window(gtk_window &&) = delete;
  gtk_window &operator=(gtk_window &&) = delete;
  virtual ~gtk_window() = default;

protected:
  void set_initial_size(const ui_size &size) {
    gtk_window_set_default_size(m_native_window.get(),
                                static_cast<int>(size.width),
                                static_cast<int>(size.height));
  }

  void set_widget(widget_ptr widget) override {
    m_widget = widget;
    m_native_widget = static_cast<GtkWidget *>(m_widget->get_native_handle());
    gtk_compat::window_set_child(m_native_window.get(), m_native_widget.get());
    gtk_compat::widget_set_visible(m_native_widget.get(), true);
  }

  //virtual widget_ptr create_widget_impl() = 0;

  window_events &events_impl() override { return m_events; }

  void dispatch_impl(dispatch_fn_t f) override { m_event_loop->dispatch(f); }

  void *get_native_handle_impl() const override {
    return m_native_window.get();
  }

  void set_title_impl(const std::string &title) override {
    gtk_window_set_title(m_native_window.get(), title.c_str());
  }

  void set_visible_impl(bool visible) override {
    gtk_compat::widget_set_visible(GTK_WIDGET(m_native_window.get()), visible);
  }

  void close_impl() override { gtk_window_close(m_native_window.get()); }

  void destroy_impl() override {
    { gtk_compat::window_destroy(m_native_window.get()); }
  }

private:
  window_events m_events;
  event_loop_ptr m_event_loop;
  gtk_ref<GtkWindow> m_native_window;
  gtk_ref<GtkWidget> m_native_widget;
  widget_ptr m_widget;
  gtk_compat::signal_connection m_close_request_conn;
  gtk_compat::signal_connection m_destroy_conn;
};

} // namespace detail
} // namespace webview

#endif
#endif // WEBVIEW_DETAIL_UI_LINUX_GTK_WINDOW_HH
