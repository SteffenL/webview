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

#ifndef WEBVIEW_HTTP_HH
#define WEBVIEW_HTTP_HH

#include "detail/promise.hh"

#include <map>
#include <string>

namespace webview {
namespace http {

class content_type {};

using header_map = std::map<std::string, std::string>;

class request {
public:
  explicit request(std::string method, std::string path) noexcept
      : m_method{std::move(method)}, m_path{std::move(path)} {}

  const std::string &get_method() const noexcept { return m_method; }
  const std::string &get_path() const noexcept { return m_path; }
  const header_map &get_headers() const noexcept { return m_headers; }

  request &set_header(const header_map::key_type &name,
                      header_map::mapped_type value) noexcept {
    m_headers[name] = std::move(value);
    return *this;
  }

private:
  std::string m_method;
  std::string m_path;
  header_map m_headers;
};

class status {
public:
  status(int code, std::string reason = {})
      : m_code{code}, m_reason{std::move(reason)} {}

  int get_code() const noexcept { return m_code; }
  const std::string &get_reason() const noexcept { return m_reason; }

private:
  int m_code{};
  std::string m_reason;
};

class response {
public:
  explicit response(class status status) noexcept : m_status{status} {}

  const status &get_status() const noexcept { return m_status; }
  const header_map &get_headers() const noexcept { return m_headers; }

  const std::string &get_content_type() const noexcept {
    return m_content_type;
  }

  response &set_header(const header_map::key_type &name,
                       header_map::mapped_type value) {
    m_headers[name] = std::move(value);
    return *this;
  }

  response &set_content_type(std::string content_type) {
    m_content_type = std::move(content_type);
    return *this;
  }

private:
  status m_status;
  header_map m_headers;
  std::string m_content_type;
};

using response_promise = detail::promise<response>;

} // namespace http
} // namespace webview

#endif // WEBVIEW_HTTP_HH
