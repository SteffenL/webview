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

#include "detail/memstream.hh"
#include "detail/promise.hh"
#include "detail/stream.hh"

#include <cstdint>
#include <limits>
#include <map>
#include <memory>
#include <string>

namespace webview {
namespace http {
namespace detail {

using namespace webview::detail;

inline const std::map<int, std::string> &status_codes() noexcept {
  static const std::map<int, std::string> codes{
      {100, "Continue"},
      {101, "Switching Protocols"},
      {102, "Processing (WebDAV)"},
      {103, "Early Hints"},
      {200, "OK"},
      {201, "Created"},
      {202, "Accepted"},
      {203, "Non-Authoritative Information"},
      {204, "No Content"},
      {205, "Reset Content"},
      {206, "Partial Content"},
      {207, "Multi-Status (WebDAV)"},
      {208, "Already Reported (WebDAV)"},
      {226, "IM Used (HTTP Delta encoding)"},
      {300, "Multiple Choices"},
      {301, "Moved Permanently"},
      {302, "Found"},
      {303, "See Other"},
      {304, "Not Modified"},
      {305, "Use Proxy Deprecated"},
      {306, "unused"},
      {307, "Temporary Redirect"},
      {308, "Permanent Redirect"},
      {400, "Bad Request"},
      {401, "Unauthorized"},
      {402, "Payment Required"},
      {403, "Forbidden"},
      {404, "Not Found"},
      {405, "Method Not Allowed"},
      {406, "Not Acceptable"},
      {407, "Proxy Authentication Required"},
      {408, "Request Timeout"},
      {409, "Conflict"},
      {410, "Gone"},
      {411, "Length Required"},
      {412, "Precondition Failed"},
      {413, "Payload Too Large"},
      {414, "URI Too Long"},
      {415, "Unsupported Media Type"},
      {416, "Range Not Satisfiable"},
      {417, "Expectation Failed"},
      {418, "I'm a teapot"},
      {421, "Misdirected Request"},
      {422, "Unprocessable Content (WebDAV)"},
      {423, "Locked (WebDAV)"},
      {424, "Failed Dependency (WebDAV)"},
      {425, "Too Early"},
      {426, "Upgrade Required"},
      {428, "Precondition Required"},
      {429, "Too Many Requests"},
      {431, "Request Header Fields Too Large"},
      {451, "Unavailable For Legal Reasons"},
      {500, "Internal Server Error"},
      {501, "Not Implemented"},
      {502, "Bad Gateway"},
      {503, "Service Unavailable"},
      {504, "Gateway Timeout"},
      {505, "HTTP Version Not Supported"},
      {506, "Variant Also Negotiates"},
      {507, "Insufficient Storage (WebDAV)"},
      {508, "Loop Detected (WebDAV)"},
      {510, "Not Extended"},
      {511, "Network Authentication Required"}};
  return codes;
}

inline const std::string &get_status_reason_by_code(int code) noexcept {
  auto found{status_codes().find(code)};
  if (found == status_codes().end()) {
    static std::string empty;
    return empty;
  }
  return found->second;
}

} // namespace detail

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
  status(int code)
      : m_code{code}, m_reason{detail::get_status_reason_by_code(code)} {}

  status(int code, std::string reason)
      : m_code{code}, m_reason{std::move(reason)} {}

  int get_code() const noexcept { return m_code; }
  const std::string &get_reason() const noexcept { return m_reason; }

private:
  int m_code{};
  std::string m_reason;
};

class content_source {
public:
  const std::string &get_content_type() const { return m_content_type; }

  detail::input_stream_ptr create_stream() const { return m_factory(); }

  static content_source string(std::string value, std::string content_type) {
    return content_source{std::bind(
                              [](std::string value_) {
                                return detail::make_string_input_stream(
                                    std::move(value_));
                              },
                              std::move(value)),
                          std::move(content_type)};
  }

  static content_source file(std::string file_path, std::string content_type) {
    return content_source{std::bind(
                              [](std::string file_path_) {
                                return detail::make_file_input_stream(
                                    file_path_);
                              },
                              file_path),
                          std::move(content_type)};
  }

protected:
  explicit content_source(std::function<detail::input_stream_ptr()> factory,
                          std::string content_type)
      : m_factory{std::move(factory)},
        m_content_type{std::move(content_type)} {}

private:
  std::function<detail::input_stream_ptr()> m_factory;
  std::string m_content_type;
};

class response {
public:
  explicit response(class status status) noexcept : m_status{status} {}

  const status &get_status() const noexcept { return m_status; }
  const header_map &get_headers() const noexcept { return m_headers; }

  const content_source &get_content_source() const noexcept {
    return m_content_source;
  }

  response &set_header(const header_map::key_type &name,
                       header_map::mapped_type value) {
    m_headers[name] = std::move(value);
    return *this;
  }

  response &set_content_source(content_source source) {
    m_content_source = std::move(source);
    return *this;
  }

private:
  status m_status;
  header_map m_headers;
  content_source m_content_source{content_source::string({}, "text/plain")};
};

using response_promise = ::webview::detail::promise<response>;

} // namespace http
} // namespace webview

#endif // WEBVIEW_HTTP_HH
