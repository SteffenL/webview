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

#ifndef WEBVIEW_DETAIL_UI_LINUX_GTK_APPLICATION_HH
#define WEBVIEW_DETAIL_UI_LINUX_GTK_APPLICATION_HH

#include "../../../../../macros.h"

#if defined(WEBVIEW_PLATFORM_LINUX) && defined(WEBVIEW_GTK)

#include "../../../../../detail/platform/linux/gtk/compat.hh"
#include "../../../../../detail/signal.hh"
#include "../../../../../types.hh"
#include "../../application_base.hh"
#include "../../event_loop_base.hh"

#include <gtk/gtk.h>

namespace webview {
namespace detail {

class gtk_application : public application_base {
public:
  explicit gtk_application(event_loop_ptr loop)
      : application_base{loop}, m_event_loop{loop} {
    if (!gtk_compat::init_check()) {
      throw exception{WEBVIEW_ERROR_UNSPECIFIED, "GTK init failed"};
    }

    m_event_loop->dispatch([=] { m_events.ready.emit(); });
  }

  virtual ~gtk_application() = default;

protected:
  application_events &events_impl() override { return m_events; }

private:
  application_events m_events;
  event_loop_ptr m_event_loop;
};

} // namespace detail
} // namespace webview

#endif
#endif // WEBVIEW_DETAIL_UI_LINUX_GTK_APPLICATION_HH
