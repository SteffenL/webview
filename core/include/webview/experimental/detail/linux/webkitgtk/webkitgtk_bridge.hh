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

#ifndef WEBVIEW_DETAIL_LINUX_WEBKITGTK_BRIDGE_HH
#define WEBVIEW_DETAIL_LINUX_WEBKITGTK_BRIDGE_HH

#include "../../../../macros.h"

#if defined(WEBVIEW_PLATFORM_LINUX) && defined(WEBVIEW_GTK)

#include "../../../../detail/json.hh"
#include "../../../../detail/platform/linux/webkitgtk/dmabuf.hh"
#include "../../../../types.hh"
#include "../../bridge_base.hh"
#include "../../event_loop_base.hh"
#include "../../user_content_manager_base.hh"
#include "../gtk/gtk_ref.hh"

#include <gtk/gtk.h>

#include <string>

#if GTK_MAJOR_VERSION >= 4

#include <jsc/jsc.h>
#include <webkit/webkit.h>

#elif GTK_MAJOR_VERSION >= 3

#include <JavaScriptCore/JavaScript.h>
#include <webkit2/webkit2.h>

#endif

namespace webview {
namespace detail {

class webkitgtk_bridge : public bridge_base {
public:
  explicit webkitgtk_bridge(user_content_manager_ptr user_content,
                            iscript_evaluator *script_evaluator)
      : bridge_base{user_content, script_evaluator} {
    add_init_script("function(message) {\n\
  return window.webkit.messageHandlers.__webview__.postMessage(message);\n\
}");
  }

  virtual ~webkitgtk_bridge() = default;
};

} // namespace detail
} // namespace webview

#endif
#endif // WEBVIEW_DETAIL_LINUX_WEBKITGTK_BRIDGE_HH
