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

#ifndef WEBVIEW_DETAIL_LINUX_WEBKITGTK_BROWSER_HH
#define WEBVIEW_DETAIL_LINUX_WEBKITGTK_BROWSER_HH

#include "../../../../macros.h"

#if defined(WEBVIEW_PLATFORM_LINUX) && defined(WEBVIEW_GTK)

#include "../../../../detail/platform/linux/webkitgtk/dmabuf.hh"
#include "../../../../types.hh"
#include "../../browser_base.hh"
#include "../../event_loop_base.hh"
#include "../gtk/gtk_ref.hh"
#include "webkitgtk_bridge.hh"
#include "webkitgtk_uri_scheme_manager.hh"
#include "webkitgtk_user_content_manager.hh"

#include <gtk/gtk.h>

#if GTK_MAJOR_VERSION >= 4

#include <jsc/jsc.h>
#include <webkit/webkit.h>

#elif GTK_MAJOR_VERSION >= 3

#include <JavaScriptCore/JavaScript.h>
#include <webkit2/webkit2.h>

#endif

namespace webview {
namespace detail {

class webkitgtk_browser : public browser_base {
public:
  explicit webkitgtk_browser(event_loop_ptr loop) : m_event_loop{loop} {
    webkit_dmabuf::apply_webkit_dmabuf_workaround();
    m_native_browser = webkit_web_view_new();

    m_native_context = webkit_web_context_get_default();
    g_object_unref(m_native_context.get());

    m_user_content.reset(new webkitgtk_user_content_manager{
        webkit_web_view_get_user_content_manager(
            WEBKIT_WEB_VIEW(m_native_browser.get()))});
    m_bridge.reset(new webkitgtk_bridge{m_user_content, this});
    m_uri_scheme_manager.reset(
        new webkitgtk_uri_scheme_manager{m_native_context});
    m_event_loop->dispatch([&] { m_events.ready.emit(); });
  }

  virtual ~webkitgtk_browser() = default;

  browser_events &events() override { return m_events; }
  void *get_native_handle() const override { return m_native_browser.get(); }
  void *get_native_embeddable() override { return m_native_browser.get(); }

  void navigate(const std::string &url) override {
    webkit_web_view_load_uri(WEBKIT_WEB_VIEW(m_native_browser.get()),
                             url.c_str());
  }

  void set_html(const std::string &html) override {
    webkit_web_view_load_html(WEBKIT_WEB_VIEW(m_native_browser.get()),
                              html.c_str(), nullptr);
  }

  void eval(const std::string &js) override {
    // URI is null before content has begun loading.
    if (!webkit_web_view_get_uri(WEBKIT_WEB_VIEW(m_native_browser.get()))) {
      return;
    }
#if (WEBKIT_MAJOR_VERSION == 2 && WEBKIT_MINOR_VERSION >= 40) ||               \
    WEBKIT_MAJOR_VERSION > 2
    webkit_web_view_evaluate_javascript(WEBKIT_WEB_VIEW(m_native_browser.get()),
                                        js.c_str(),
                                        static_cast<gssize>(js.size()), nullptr,
                                        nullptr, nullptr, nullptr, nullptr);
#else
    webkit_web_view_run_javascript(WEBKIT_WEB_VIEW(m_native_browser.get()),
                                   js.c_str(), nullptr, nullptr, nullptr);
#endif
  }

  user_content_manager_ptr user_content() override { return m_user_content; }
  bridge_ptr bridge() override { return m_bridge; }
  uri_scheme_manager_ptr uri_schemes() override { return m_uri_scheme_manager; }

private:
  browser_events m_events;
  event_loop_ptr m_event_loop;
  gtk_ref<GtkWidget> m_native_browser;
  gtk_ref<WebKitWebContext> m_native_context;
  user_content_manager_ptr m_user_content;
  bridge_ptr m_bridge;
  uri_scheme_manager_ptr m_uri_scheme_manager;
};

} // namespace detail
} // namespace webview

#endif
#endif // WEBVIEW_DETAIL_LINUX_WEBKITGTK_BROWSER_HH
