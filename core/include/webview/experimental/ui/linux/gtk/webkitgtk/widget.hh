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

#if !defined(WEBVIEW_DETAIL_UI_LINUX_WEBKITGTK_WIDGET_HH) &&                   \
    defined(WEBVIEW_PLATFORM_LINUX) && defined(WEBVIEW_GTK)
#define WEBVIEW_DETAIL_UI_LINUX_WEBKITGTK_WIDGET_HH

#include "../../../../../detail/platform/linux/gtk/compat.hh"
#include "../../../../../detail/platform/linux/webkitgtk/dmabuf.hh"
#include "../../../../../detail/signal.hh"
#include "../../../../../types.hh"
#include "../../../event_loop.hh"
#include "../ref.hh"

#include <gtk/gtk.h>

#if GTK_MAJOR_VERSION >= 4

#include <jsc/jsc.h>
#include <webkit/webkit.h>

#elif GTK_MAJOR_VERSION >= 3

#include <JavaScriptCore/JavaScript.h>
#include <webkit2/webkit2.h>

#endif

#include <memory>

namespace webview {
namespace detail {

class webkitgtk_widget {
public:
  class events_t {
  public:
    signal<void()> ready;
  };

  webkitgtk_widget(std::shared_ptr<event_loop> loop) : m_event_loop{loop} {
    webkit_dmabuf::apply_webkit_dmabuf_workaround();
    m_native_widget = webkit_web_view_new();
    m_event_loop->dispatch([&] { m_events.ready.emit(); });
  }

  webkitgtk_widget(const webkitgtk_widget &) = delete;
  webkitgtk_widget &operator=(const webkitgtk_widget &) = delete;
  webkitgtk_widget(webkitgtk_widget &&) = delete;
  webkitgtk_widget &operator=(webkitgtk_widget &&) = delete;
  ~webkitgtk_widget() = default;

  events_t &events() { return m_events; }
  void *get_native_handle() const { return m_native_widget.get(); }

  void navigate(const std::string &url) {
    webkit_web_view_load_uri(WEBKIT_WEB_VIEW(m_native_widget.get()),
                             url.c_str());
  }

  void set_html(const std::string &html) {
    webkit_web_view_load_html(WEBKIT_WEB_VIEW(m_native_widget.get()),
                              html.c_str(), nullptr);
  }

private:
  events_t m_events;
  std::shared_ptr<event_loop> m_event_loop;
  gtk_ref<GtkWidget> m_native_widget;
};

using widget_impl = webkitgtk_widget;

} // namespace detail
} // namespace webview

#endif // WEBVIEW_DETAIL_UI_LINUX_WEBKITGTK_WIDGET_HH
