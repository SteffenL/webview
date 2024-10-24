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

#ifndef WEBVIEW_DETAIL_LINUX_GTK_WIDGET_HH
#define WEBVIEW_DETAIL_LINUX_GTK_WIDGET_HH

#include "../../../../macros.h"

#if defined(WEBVIEW_PLATFORM_LINUX) && defined(WEBVIEW_GTK)

#include "../../../../detail/platform/linux/gtk/compat.hh"
#include "../../widget_base.hh"
#include "../gtk/gtk_ref.hh"

#include <gtk/gtk.h>

namespace webview {
namespace detail {

class gtk_widget : public widget_base {
public:
  explicit gtk_widget()
      : m_native_widget{gtk_box_new(GTK_ORIENTATION_VERTICAL, 0)} {}

  virtual ~gtk_widget() = default;

  void *get_native_handle() const override { return m_native_widget.get(); }

  void embed(void *native_embeddable) override {
    m_native_child = static_cast<GtkWidget *>(native_embeddable);
    gtk_box_pack_start(GTK_BOX(m_native_widget.get()), m_native_child.get(),
                       TRUE, TRUE, 0);
    gtk_compat::widget_set_visible(m_native_child.get(), true);
  }

private:
  gtk_ref<GtkWidget> m_native_widget;
  gtk_ref<GtkWidget> m_native_child;
};

} // namespace detail
} // namespace webview

#endif
#endif // WEBVIEW_DETAIL_LINUX_GTK_WIDGET_HH
