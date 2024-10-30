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

#ifndef WEBVIEW_DETAIL_LINUX_G_INPUT_STREAM_WRAPPER_HH
#define WEBVIEW_DETAIL_LINUX_G_INPUT_STREAM_WRAPPER_HH

#include "../../../../macros.h"

#if defined(WEBVIEW_PLATFORM_LINUX) && defined(WEBVIEW_GTK)

#include <gtk/gtk.h>

#include <istream>
#include <memory>
#include <sstream>

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
inline gssize webview_g_input_stream_wrapper_read_fn(
    GInputStream *stream, void *buffer, gsize count,
    GCancellable * /*cancellable*/, GError **error) {
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
}

// Ref.: https://docs.gtk.org/gio/vfunc.InputStream.skip.html
inline gssize
webview_g_input_stream_wrapper_skip(GInputStream *stream, gsize count,
                                    GCancellable * /*cancellable*/,
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
inline gboolean webview_g_input_stream_wrapper_close_fn(
    GInputStream *stream, GCancellable * /*cancellable*/, GError **error) {
  auto *self{WEBVIEW_G_INPUT_STREAM_WRAPPER(stream)};
  if (error) {
    *error = nullptr;
  }
  self->priv->stream.reset();
  return TRUE;
}

inline void webview_g_input_stream_wrapper_finalize(GObject *object) {
  auto *self{WEBVIEW_G_INPUT_STREAM_WRAPPER(object)};

  delete self->priv;
  self->priv = nullptr;

  auto *object_class{
      G_OBJECT_CLASS(webview_g_input_stream_wrapper_parent_class)};
  object_class->finalize(object);
}

inline void webview_g_input_stream_wrapper_class_init(
    WebViewGInputStreamWrapperClass *class_) {
  auto *object_class{G_OBJECT_CLASS(class_)};
  object_class->finalize = webview_g_input_stream_wrapper_finalize;

  auto *stream_class{G_INPUT_STREAM_CLASS(class_)};
  stream_class->read_fn = webview_g_input_stream_wrapper_read_fn;
  stream_class->skip = webview_g_input_stream_wrapper_skip;
  stream_class->close_fn = webview_g_input_stream_wrapper_close_fn;
}

inline void
webview_g_input_stream_wrapper_init(WebViewGInputStreamWrapper *self) {
  self->priv = new WebViewGInputStreamWrapperPrivate{};
}

template <typename T>
void webview_g_input_stream_wrapper_load_string(
    WebViewGInputStreamWrapper *self, T &&content) {
  self->priv->stream.reset(new std::istringstream{std::forward<T>(content)});
}

template <typename T>
void webview_g_input_stream_wrapper_set_stream(WebViewGInputStreamWrapper *self,
                                               T &&stream) {
  self->priv->stream = std::forward<T>(stream);
}

} // namespace detail
} // namespace webview

#endif
#endif // WEBVIEW_DETAIL_LINUX_G_INPUT_STREAM_WRAPPER_HH
