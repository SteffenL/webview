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

#ifndef WEBVIEW_UI_LINUX_GTK_RUN_LOOP
#define WEBVIEW_UI_LINUX_GTK_RUN_LOOP

#include "../../../../macros.h"

#if defined(WEBVIEW_PLATFORM_LINUX) && defined(WEBVIEW_GTK)

#include "../../../../types.hh"
#include "../../event_loop_base.hh"

#include <gtk/gtk.h>

namespace webview {
namespace detail {

class gtk_event_loop : public event_loop_base {
protected:
  void iterate_impl(bool block) override {
    g_main_context_iteration(nullptr, block ? TRUE : FALSE);
  }

  void dispatch_impl(dispatch_fn_t f) override {
    g_idle_add_full(G_PRIORITY_HIGH_IDLE, (GSourceFunc)([](void *fn) -> int {
                      (*static_cast<dispatch_fn_t *>(fn))();
                      return G_SOURCE_REMOVE;
                    }),
                    new dispatch_fn_t(f),
                    [](void *fn) { delete static_cast<dispatch_fn_t *>(fn); });
  }
};

} // namespace detail
} // namespace webview

#endif
#endif // WEBVIEW_UI_LINUX_GTK_RUN_LOOP
