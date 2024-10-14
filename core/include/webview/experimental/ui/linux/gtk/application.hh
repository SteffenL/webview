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

#include "../../../../detail/platform/linux/gtk/compat.hh"
#include "../../../../detail/signal.hh"
#include "../../../../types.hh"
#include "../../event_loop.hh"

#include <gtk/gtk.h>

#include <memory>

namespace webview {
namespace detail {

class gtk_application {
public:
  class events_t {
  public:
    signal<void()> ready;
  };

  gtk_application(std::shared_ptr<event_loop> loop) : m_event_loop{loop} {
    if (!gtk_compat::init_check()) {
      throw exception{WEBVIEW_ERROR_UNSPECIFIED, "GTK init failed"};
    }

    m_event_loop->dispatch([=] { m_events.ready.emit(); });
  }

  gtk_application(const gtk_application &) = delete;
  gtk_application &operator=(const gtk_application &) = delete;
  gtk_application(gtk_application &&) = delete;
  gtk_application &operator=(gtk_application &&) = delete;
  ~gtk_application() = default;

  events_t &events() { return m_events; }

private:
  events_t m_events;
  std::shared_ptr<event_loop> m_event_loop;
};

using application_impl = gtk_application;

} // namespace detail
} // namespace webview

#endif // WEBVIEW_DETAIL_UI_LINUX_GTK_APPLICATION_HH
