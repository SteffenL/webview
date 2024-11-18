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

#ifndef WEBVIEW_DETAIL_LINUX_WEBKITGTK_USER_CONTENT_MANAGER_HH
#define WEBVIEW_DETAIL_LINUX_WEBKITGTK_USER_CONTENT_MANAGER_HH

#include "../../../../macros.h"

#if defined(WEBVIEW_PLATFORM_LINUX) && defined(WEBVIEW_GTK)

#include "../../../../detail/platform/linux/webkitgtk/compat.hh"
#include "../../user_content_manager_base.hh"
#include "../gtk/gtk_ref.hh"
#include "webkitgtk_user_script.hh"

#include <gtk/gtk.h>

#include <cassert>
#include <list>
#include <stdexcept>

#if GTK_MAJOR_VERSION >= 4
#include <webkit/webkit.h>
#elif GTK_MAJOR_VERSION >= 3
#include <webkit2/webkit2.h>

#endif

namespace webview {
namespace detail {

class webkitgtk_user_content_manager : public user_content_manager_base {
public:
  explicit webkitgtk_user_content_manager(
      gtk_ref<WebKitUserContentManager> native_ucm)
      : m_native_ucm{native_ucm} {
    webkitgtk_compat::connect_script_message_received(
        m_native_ucm.get(), "__webview__",
        [=](WebKitUserContentManager *, const std::string &payload) {
          events().message_received.emit(this, payload);
        });
    webkitgtk_compat::user_content_manager_register_script_message_handler(
        m_native_ucm.get(), "__webview__");
  }

  webkitgtk_user_content_manager(const webkitgtk_user_content_manager &) =
      delete;

  webkitgtk_user_content_manager &
  operator=(const webkitgtk_user_content_manager &) = delete;

  webkitgtk_user_content_manager(webkitgtk_user_content_manager &&other) =
      default;

  webkitgtk_user_content_manager &
  operator=(webkitgtk_user_content_manager &&other) = default;

  virtual ~webkitgtk_user_content_manager() = default;

  user_content_manager_events &events() override { return m_events; }

  user_script_ptr add_script(const std::string &code,
                             user_script_injection_time where) override {
    return add_script_impl(code, where);
  }

  user_script_ptr add_script(std::string &&code,
                             user_script_injection_time where) override {
    return add_script_impl(std::move(code), where);
  }

  user_script_ptr replace_script(user_script_ptr old_script,
                                 const std::string &new_code) override {
    auto existing_scripts{m_scripts};
    remove_all_scripts();
    user_script_ptr recreated_script;
    for (auto &script : existing_scripts) {
      auto is_old_script = script->equals(old_script);
      script = add_script_impl(is_old_script ? new_code : script->get_code(),
                               script->get_injection_time());
      if (is_old_script) {
        recreated_script = script;
      }
    }
    return recreated_script;
  }

  void remove_script(user_script_ptr script) override {
    webkit_user_content_manager_remove_all_scripts(m_native_ucm.get());
    for (auto &script_ : m_scripts) {
      if (!script_->equals(script)) {
        script_ =
            add_script_impl(script->get_code(), script->get_injection_time());
      }
    }
  }

  void remove_all_scripts() override {
    webkit_user_content_manager_remove_all_scripts(m_native_ucm.get());
    m_scripts.clear();
  }

private:
  template <typename T>
  user_script_ptr add_script_impl(T &&code, user_script_injection_time where) {
    auto *native_script{webkit_user_script_new(
        code.c_str(), WEBKIT_USER_CONTENT_INJECT_TOP_FRAME, map(where), nullptr,
        nullptr)};
    auto script{user_script_ptr{new webkitgtk_user_script{
        std::forward<T>(code), native_script, where}}};
    m_scripts.push_back(script);
    webkit_user_content_manager_add_script(m_native_ucm.get(), native_script);
    webkit_user_script_unref(native_script);
    return script;
  }

  static WebKitUserScriptInjectionTime map(user_script_injection_time where) {
    switch (where) {
    case user_script_injection_time::start:
      return WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START;
    case user_script_injection_time::end:
      return WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_END;
    }
    assert(!!"Invalid enum value");
    throw std::logic_error{"Invalid enum value"};
  }

  user_content_manager_events m_events;
  gtk_ref<WebKitUserContentManager> m_native_ucm;
  std::list<user_script_ptr> m_scripts;
};

} // namespace detail
} // namespace webview

#endif
#endif // WEBVIEW_DETAIL_LINUX_WEBKITGTK_USER_CONTENT_MANAGER_HH
