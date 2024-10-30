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

#ifndef WEBVIEW_DETAIL_LINUX_WEBKITGTK_URI_SCHEME_MANAGER_HH
#define WEBVIEW_DETAIL_LINUX_WEBKITGTK_URI_SCHEME_MANAGER_HH

#include "../../../../macros.h"

#if defined(WEBVIEW_PLATFORM_LINUX) && defined(WEBVIEW_GTK)

#include "../../uri_scheme_manager_base.hh"
#include "../gtk/g_input_stream_wrapper.hh"
#include "../gtk/gtk_ref.hh"

#include <gtk/gtk.h>

#include <string>

#if GTK_MAJOR_VERSION >= 4
#include <webkit/webkit.h>
#elif GTK_MAJOR_VERSION >= 3
#include <webkit2/webkit2.h>

#endif

namespace webview {
namespace detail {

class webkitgtk_uri_scheme_manager : public uri_scheme_manager_base {
public:
  explicit webkitgtk_uri_scheme_manager(gtk_ref<WebKitWebContext> context)
      : m_context{std::move(context)} {}

  virtual ~webkitgtk_uri_scheme_manager() = default;

  void bind(const std::string &name, handler_type handler) override {
    auto fn{+[](WebKitURISchemeRequest *native_request, gpointer user_data) {
      // FIXME: dangling self pointer after move
      //auto *self{static_cast<webkitgtk_uri_scheme_manager *>(user_data)};
      auto &handler_{*static_cast<handler_type *>(user_data)};
      http::request request{
          webkit_uri_scheme_request_get_http_method(native_request),
          webkit_uri_scheme_request_get_path(native_request)};
      copy_headers_from_native_request(native_request, request);
      //std::string uri{webkit_uri_scheme_request_get_uri(request)};
      //auto *body_stream{webkit_uri_scheme_request_get_http_body(request)};
      //http::response response{0};
      handler_(
          request,
          http::response_promise{
              [=](http::response response) {
                gtk_ref<WebViewGInputStreamWrapper> stream_wrapper{
                    WEBVIEW_G_INPUT_STREAM_WRAPPER(g_object_new(
                        WEBVIEW_TYPE_G_INPUT_STREAM_WRAPPER, nullptr)),
                    false};
                webview_g_input_stream_wrapper_init(stream_wrapper.get());
                const auto& source{response.get_content_source()};
                webview_g_input_stream_wrapper_set_stream(stream_wrapper.get(), source.create_stream());

                gtk_ref<WebKitURISchemeResponse> native_response{
                    webkit_uri_scheme_response_new(
                        G_INPUT_STREAM(stream_wrapper.get()), -1),
                    false};
                webkit_uri_scheme_response_set_status(
                    native_response.get(),
                    static_cast<guint>(response.get_status().get_code()),
                    response.get_status().get_reason().c_str());
                webkit_uri_scheme_response_set_content_type(
                    native_response.get(), source.get_content_type().c_str());
                webkit_uri_scheme_request_finish_with_response(
                    native_request, native_response.get());
              },
              [=](http::response) {
                webkit_uri_scheme_request_finish_error(native_request, nullptr);
              }});
    }};
    webkit_web_context_register_uri_scheme(
        m_context.get(), name.c_str(), fn, new handler_type{std::move(handler)},
        +[](void *p) { delete static_cast<handler_type *>(p); });
    auto *security_manager{
        webkit_web_context_get_security_manager(m_context.get())};
    webkit_security_manager_register_uri_scheme_as_secure(security_manager,
                                                          name.c_str());
  }

  void unbind(const std::string &name) override {
    // not supported by WebKitGTK?
  }

private:
  static void copy_headers_from_native_request(WebKitURISchemeRequest *from,
                                               http::request &to) {
    auto *from_headers{webkit_uri_scheme_request_get_http_headers(from)};
    soup_message_headers_foreach(
        from_headers,
        +[](const char *name, const char *value, gpointer user_data) {
          static_cast<http::request *>(user_data)->set_header(name, value);
        },
        &to);
  }

  gtk_ref<WebKitWebContext> m_context;
};

} // namespace detail
} // namespace webview

#endif
#endif // WEBVIEW_DETAIL_LINUX_WEBKITGTK_URI_SCHEME_MANAGER_HH
