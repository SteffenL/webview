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

#include "../../../../detail/json.hh"
#include "../../../../detail/platform/linux/webkitgtk/dmabuf.hh"
#include "../../../../types.hh"
#include "../../event_loop_base.hh"
#include "../../uri_scheme_manager_base.hh"
#include "../../user_content_manager_base.hh"
#include "../gtk/gtk_ref.hh"

#include <gtk/gtk.h>

#include <istream>
#include <memory>
#include <sstream>
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

#define WEBVIEW_TYPE_G_INPUT_STREAM_WRAPPER                                    \
  webview_g_input_stream_wrapper_get_type()

// Ref.: https://docs.gtk.org/gobject/func.DECLARE_FINAL_TYPE.html
inline G_DECLARE_FINAL_TYPE(WebViewGInputStreamWrapper,
                            webview_g_input_stream_wrapper, WEBVIEW,
                            G_INPUT_STREAM_WRAPPER, GInputStream);

struct WebViewGInputStreamWrapperPrivate {
  std::unique_ptr<std::istream> stream;
};

struct _WebViewGInputStreamWrapper {
  GInputStream parent;
  WebViewGInputStreamWrapperPrivate *priv;
};

G_DEFINE_TYPE(WebViewGInputStreamWrapper, webview_g_input_stream_wrapper,
              G_TYPE_INPUT_STREAM)

// Ref.: https://docs.gtk.org/gio/vfunc.InputStream.read_fn.html
// Ref.: https://docs.gtk.org/gio/method.InputStream.read.html
inline gssize webview_g_input_stream_wrapper_read(GInputStream *stream,
                                                  void *buffer, gsize count,
                                                  GCancellable *cancellable,
                                                  GError **error) {
  auto *self{WEBVIEW_G_INPUT_STREAM_WRAPPER(stream)};
  if (error) {
    *error = nullptr;
  }
  if (count == 0) {
    return 0;
  }
  auto &stream_{*self->priv->stream};
  if (!stream_.good()) {
    if (stream_.eof()) {
      return 0;
    }
    if (error) {
      *error = g_error_new(G_IO_ERROR, G_IO_ERROR_FAILED, "Stream is no good");
    }
    return -1;
  }
  using char_type =
      typename std::remove_reference<decltype(stream_)>::type::char_type;
  stream_.read(static_cast<char_type *>(buffer),
               static_cast<std::streamsize>(count));
  return static_cast<gssize>(stream_.gcount());

  //  // Your custom read logic here. For example, you might read from a file or network socket.
  //  // Replace the following placeholder code with actual implementation:
  //  static gsize total_read = 0;
  //  const char *data = "Hello, this is some data to be read.";
  //  gsize len = strlen(data);
  //
  //  if (total_read >= len) {
  //    return -1; // End of stream.
  //  }
  //
  //  gsize bytes_to_copy = MIN(count, len - total_read);
  //  memcpy(buffer, data + total_read, bytes_to_copy);
  //  total_read += bytes_to_copy;
  //
  //  return static_cast<gssize>(bytes_to_copy);
}

// Ref.: https://docs.gtk.org/gio/vfunc.InputStream.skip.html
inline gssize webview_g_input_stream_wrapper_skip(GInputStream *stream,
                                                  gsize count,
                                                  GCancellable *cancellable,
                                                  GError **error) {
  auto *self{WEBVIEW_G_INPUT_STREAM_WRAPPER(stream)};
  if (error) {
    *error = nullptr;
  }
  auto &stream_{*self->priv->stream};
  auto pos_before{stream_.tellg()};
  stream_.seekg(static_cast<gssize>(count));
  if (!stream_.good()) {
    if (error) {
      *error = g_error_new(G_IO_ERROR, G_IO_ERROR_UNKNOWN, "Stream is no good");
    }
    return -1;
  }
  auto pos_after{stream_.tellg()};
  auto distance{pos_after - pos_before};
  return static_cast<gssize>(distance);
}

// Ref.: https://docs.gtk.org/gio/vfunc.InputStream.close_fn.html
inline gboolean webview_g_input_stream_wrapper_close(GInputStream *stream,
                                                     GCancellable *cancellable,
                                                     GError **error) {
  auto *self{WEBVIEW_G_INPUT_STREAM_WRAPPER(stream)};
  if (error) {
    *error = nullptr;
  }
  delete self->priv;
  self->priv = nullptr;
  // TODO: Is the return value correct? Docs don't say.
  return TRUE;
}

inline void webview_g_input_stream_wrapper_class_init(
    WebViewGInputStreamWrapperClass *class_) {
  auto *stream_class{G_INPUT_STREAM_CLASS(class_)};
  stream_class->read_fn = webview_g_input_stream_wrapper_read;
  stream_class->skip = webview_g_input_stream_wrapper_skip;
  stream_class->close_fn = webview_g_input_stream_wrapper_close;
}

inline void
webview_g_input_stream_wrapper_init(WebViewGInputStreamWrapper *self) {
  self->priv = new WebViewGInputStreamWrapperPrivate{};
  auto on_destroy{
      +[](WebViewGInputStreamWrapper *self_, gpointer /*user_data*/) {
        delete self_->priv;
        self_->priv = nullptr;
      }};
  g_signal_connect(G_OBJECT(self), "destroy", G_CALLBACK(on_destroy), nullptr);
}

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
                        WEBVIEW_TYPE_G_INPUT_STREAM_WRAPPER, nullptr))};
                webview_g_input_stream_wrapper_init(stream_wrapper.get());
                //// TODO: pick stream type based on what's in the response object
                //stream_wrapper->priv->stream.reset(
                //    new std::stringstream{"Hello"});
                gint64 stream_length{-1};
                gtk_ref<WebKitURISchemeResponse> response_ref{
                    webkit_uri_scheme_response_new(
                        G_INPUT_STREAM(stream_wrapper.get()), stream_length)};
                webkit_uri_scheme_response_set_status(
                    response_ref.get(),
                    static_cast<guint>(response.get_status().get_code()),
                    response.get_status().get_reason().c_str());
                webkit_uri_scheme_request_finish_with_response(
                    native_request, response_ref.get());
              },
              [=](http::response) {
                webkit_uri_scheme_request_finish_error(native_request, nullptr);
              }});
    }};
    webkit_web_context_register_uri_scheme(
        m_context.get(), name.c_str(), fn, new handler_type{std::move(handler)},
        +[](void *p) { delete static_cast<handler_type *>(p); });
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
    soup_message_headers_unref(from_headers);
  }

  gtk_ref<WebKitWebContext> m_context;
};

} // namespace detail
} // namespace webview

#endif
#endif // WEBVIEW_DETAIL_LINUX_WEBKITGTK_URI_SCHEME_MANAGER_HH
