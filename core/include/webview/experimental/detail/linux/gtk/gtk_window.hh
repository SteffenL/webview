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

#ifndef WEBVIEW_DETAIL_LINUX_GTK_WINDOW_HH
#define WEBVIEW_DETAIL_LINUX_GTK_WINDOW_HH

#include "../../../../macros.h"

#if defined(WEBVIEW_PLATFORM_LINUX) && defined(WEBVIEW_GTK)

#include "../../../../detail/platform/linux/gtk/compat.hh"
#include "../../../primitives.h"
#include "../../window_base.hh"
#include "gtk_ref.hh"

#include <gtk/gtk.h>

namespace webview {
namespace detail {

class gtk_window : public window_base {
  struct self_state_t {
    iwindow *self;
  };

public:
  explicit gtk_window()
      : m_self_state{new self_state_t{this}},
        m_native_window{GTK_WINDOW(gtk_compat::window_new())} {
    //m_events.ready.emit(static_cast<Self *>(this));
  }

  gtk_window(const gtk_window &) = delete;
  gtk_window &operator=(const gtk_window &) = delete;
  gtk_window(gtk_window &&other) noexcept { *this = std::move(other); }

  gtk_window &operator=(gtk_window &&other) noexcept {
    if (this != &other) {
      m_self_state = std::move(other.m_self_state);
      m_self_state->self = this;

      m_native_window = std::move(other.m_native_window);
      m_native_widget = std::move(other.m_native_widget);
      m_close_request_conn = std::move(other.m_close_request_conn);
      m_destroy_conn = std::move(other.m_destroy_conn);
    }
    return *this;
  }

  virtual ~gtk_window() = default;

  void set_title(const std::string &title) override {
    gtk_window_set_title(m_native_window.get(), title.c_str());
  }

  void set_visible(bool visible) override {
    gtk_compat::widget_set_visible(GTK_WIDGET(m_native_window.get()), visible);
  }

  void close() override { gtk_window_close(m_native_window.get()); }
  void destroy() override { gtk_compat::window_destroy(m_native_window.get()); }
  void *get_native_handle() const override { return m_native_window.get(); }

protected:
  void set_initial_size(const ui_size &size) {
    gtk_window_set_default_size(m_native_window.get(),
                                static_cast<int>(size.width),
                                static_cast<int>(size.height));
  }

  void embed(void *native_embeddable) override {
    m_native_widget = static_cast<GtkWidget *>(native_embeddable);
    gtk_compat::window_set_child(m_native_window.get(), m_native_widget.get());
    gtk_compat::widget_set_visible(m_native_widget.get(), true);
  }

  void bind_native_events() noexcept {
    auto *state{m_self_state.get()};
    m_close_request_conn = gtk_compat::connect_window_close_request(
        m_native_window.get(),
        [state] { state->self->events().close_requested.emit(state->self); });
    m_destroy_conn = gtk_compat::connect_widget_destroy(
        GTK_WIDGET(m_native_window.get()),
        [state] { state->self->events().destroy.emit(state->self); });
  }

private:
  std::unique_ptr<self_state_t> m_self_state;
  gtk_ref<GtkWindow> m_native_window;
  gtk_ref<GtkWidget> m_native_widget;
  gtk_compat::signal_connection m_close_request_conn;
  gtk_compat::signal_connection m_destroy_conn;
};

} // namespace detail
} // namespace webview

#endif
#endif // WEBVIEW_DETAIL_LINUX_GTK_WINDOW_HH
