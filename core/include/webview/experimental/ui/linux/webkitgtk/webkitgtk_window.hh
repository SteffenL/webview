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

#ifndef WEBVIEW_DETAIL_UI_LINUX_WEBKITGTK_WINDOW_HH
#define WEBVIEW_DETAIL_UI_LINUX_WEBKITGTK_WINDOW_HH

#include "../../../../macros.h"

#if defined(WEBVIEW_PLATFORM_LINUX) && defined(WEBVIEW_GTK)

#include "../../../../detail/platform/linux/gtk/compat.hh"
#include "../../../../detail/signal.hh"
#include "../../../../types.hh"
#include "../../event_loop_base.hh"
#include "../../primitives.h"
//#include "../../widget_base.hh"
//#include "../../widget.hh"
#include "../gtk/gtk_ref.hh"
#include "../gtk/gtk_window.hh"

#include <gtk/gtk.h>

#include <memory>

namespace webview {
namespace detail {

class webkitgtk_window : public gtk_window {
public:
  explicit webkitgtk_window(event_loop_ptr loop) : gtk_window{loop} {}

protected:
  void add_widget() override {
    //  m_widget = create_widget_impl();
    //  m_native_widget = static_cast<GtkWidget *>(m_widget->get_native_handle());
    //  gtk_compat::window_set_child(
    //      m_native_window.get(), static_cast<GtkWidget *>(m_native_widget.get()));
    //  gtk_compat::widget_set_visible(
    //      static_cast<GtkWidget *>(m_native_widget.get()), true);
  }
};

} // namespace detail
} // namespace webview

#endif
#endif // WEBVIEW_DETAIL_UI_LINUX_WEBKITGTK_WINDOW_HH
