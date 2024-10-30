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

#ifndef WEBVIEW_DETAIL_URI_SCHEME_MANAGER_BASE_HH
#define WEBVIEW_DETAIL_URI_SCHEME_MANAGER_BASE_HH

#include "../http.hh"

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace webview {

class iuri_scheme_manager;

class iuri_scheme_manager {
public:
  using handler_type = std::function<void(const http::request &request,
                                          http::response_promise response)>;

  virtual ~iuri_scheme_manager() = default;

  virtual void bind(const std::string &name, handler_type handler) = 0;
  virtual void unbind(const std::string &name) = 0;
};

using uri_scheme_manager_ptr = std::shared_ptr<iuri_scheme_manager>;

namespace detail {

class uri_scheme_manager_base : public iuri_scheme_manager {
public:
  virtual ~uri_scheme_manager_base() = default;

private:
  std::unordered_map<std::string, handler_type> m_handlers;
};

} // namespace detail
} // namespace webview

#endif // WEBVIEW_DETAIL_URI_SCHEME_MANAGER_BASE_HH
