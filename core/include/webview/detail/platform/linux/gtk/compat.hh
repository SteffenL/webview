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

#ifndef WEBVIEW_PLATFORM_LINUX_GTK_COMPAT_HH
#define WEBVIEW_PLATFORM_LINUX_GTK_COMPAT_HH

#include "../../../../macros.h"

#if defined(WEBVIEW_PLATFORM_LINUX) && defined(WEBVIEW_GTK)

#include <functional>

#include <gtk/gtk.h>

#if GTK_MAJOR_VERSION >= 4

#ifdef GDK_WINDOWING_X11
#include <gdk/x11/gdkx.h>
#endif

#elif GTK_MAJOR_VERSION >= 3

#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#endif

#endif

namespace webview {
namespace detail {

/**
 * GTK compatibility helper class.
 */
class gtk_compat {
public:
  class signal_connection {
  public:
    signal_connection() = default;
    explicit signal_connection(std::function<void()> deleter)
        : m_deleter{deleter} {}
    signal_connection(const signal_connection &) = delete;
    signal_connection &operator=(const signal_connection &) = delete;
    signal_connection(signal_connection &&other) noexcept {
      *this = std::move(other);
    }

    signal_connection &operator=(signal_connection &&other) noexcept {
      if (this != &other) {
        m_deleter = other.m_deleter;
        other.m_deleter = {};
      }
      return *this;
    }

    ~signal_connection() {
      if (m_deleter) {
        m_deleter();
      }
    }

  private:
    std::function<void()> m_deleter;
  };

  static gboolean init_check() {
#if GTK_MAJOR_VERSION >= 4
    return gtk_init_check();
#else
    return gtk_init_check(nullptr, nullptr);
#endif
  }

  static GtkWidget *window_new() {
#if GTK_MAJOR_VERSION >= 4
    return gtk_window_new();
#else
    return gtk_window_new(GTK_WINDOW_TOPLEVEL);
#endif
  }

  static void window_set_child(GtkWindow *window, GtkWidget *widget) {
#if GTK_MAJOR_VERSION >= 4
    gtk_window_set_child(window, widget);
#else
    gtk_container_add(GTK_CONTAINER(window), widget);
#endif
  }

  static void window_remove_child(GtkWindow *window, GtkWidget *widget) {
#if GTK_MAJOR_VERSION >= 4
    if (gtk_window_get_child(window) == widget) {
      gtk_window_set_child(window, nullptr);
    }
#else
    gtk_container_remove(GTK_CONTAINER(window), widget);
#endif
  }

  static void widget_set_visible(GtkWidget *widget, bool visible) {
#if GTK_MAJOR_VERSION >= 4
    gtk_widget_set_visible(widget, visible ? TRUE : FALSE);
#else
    if (visible) {
      gtk_widget_show(widget);
    } else {
      gtk_widget_hide(widget);
    }
#endif
  }

  static void window_set_size(GtkWindow *window, int width, int height) {
#if GTK_MAJOR_VERSION >= 4
    gtk_window_set_default_size(window, width, height);
#else
    gtk_window_resize(window, width, height);
#endif
  }

  static void window_set_max_size(GtkWindow *window, int width, int height) {
// X11-specific features are available in GTK 3 but not GTK 4
#if GTK_MAJOR_VERSION < 4
    GdkGeometry g{};
    g.max_width = width;
    g.max_height = height;
    GdkWindowHints h = GDK_HINT_MAX_SIZE;
    gtk_window_set_geometry_hints(GTK_WINDOW(window), nullptr, &g, h);
#else
    // Avoid "unused parameter" warnings
    (void)window;
    (void)width;
    (void)height;
#endif
  }

  static void window_destroy(GtkWindow *window) {
#if GTK_MAJOR_VERSION >= 4
    gtk_window_destroy(window);
#else
    gtk_widget_destroy(GTK_WIDGET(window));
#endif
  }

#if __cplusplus >= 201703L
  [[nodiscard]]
#endif
  static signal_connection
  connect_window_close_request(GtkWindow *window,
                               std::function<void()> handler) {
    auto *handler_ptr{new decltype(handler){handler}};
#if GTK_MAJOR_VERSION >= 4
    g_signal_connect(
        G_OBJECT(window), "close-request",
        G_CALLBACK(+[](GtkWidget *, gpointer user_arg) -> gboolean {
          (*static_cast<decltype(handler) *>(user_arg))();
          return TRUE;
        }),
        handler_ptr);
#else
    g_signal_connect(
        G_OBJECT(window), "delete-event",
        G_CALLBACK(+[](GtkWidget *, GdkEvent *, gpointer user_arg) -> gboolean {
          (*static_cast<decltype(handler) *>(user_arg))();
          return TRUE;
        }),
        handler_ptr);
#endif
    return signal_connection{[=] {
      g_signal_handlers_disconnect_by_data(G_OBJECT(window), handler_ptr);
      delete handler_ptr;
    }};
  }

#if __cplusplus >= 201703L
  [[nodiscard]]
#endif
  static signal_connection
  connect_widget_destroy(GtkWidget *widget, std::function<void()> handler) {
    auto *handler_ptr{new decltype(handler){handler}};
    g_signal_connect(
        G_OBJECT(widget), "destroy",
        G_CALLBACK(+[](GtkWidget *, gpointer user_arg) -> gboolean {
          (*static_cast<decltype(handler) *>(user_arg))();
          return TRUE;
        }),
        handler_ptr);
    return signal_connection{[=] {
      g_signal_handlers_disconnect_by_data(G_OBJECT(widget), handler_ptr);
      delete handler_ptr;
    }};
  }
};

} // namespace detail
} // namespace webview

#endif
#endif // WEBVIEW_PLATFORM_LINUX_GTK_COMPAT_HH
