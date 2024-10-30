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

#ifndef WEBVIEW_DETAIL_STREAM_HH
#define WEBVIEW_DETAIL_STREAM_HH

#include "memstream.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace webview {
namespace detail {

class iinput_stream {
public:
  virtual ~iinput_stream() = default;

  virtual intptr_t read(void *buffer, intptr_t count) = 0;
  virtual intptr_t seek(intptr_t count) = 0;
  virtual void close() = 0;
};

using input_stream_ptr = std::shared_ptr<iinput_stream>;

class string_input_stream : public iinput_stream {
public:
  string_input_stream(std::string value)
      : m_value{std::move(value)}, m_stream{m_value.data(), m_value.size()} {}

  intptr_t read(void *buffer, intptr_t count) override {
    m_stream.read(static_cast<char *>(buffer), count);
  }

  intptr_t seek(intptr_t count) override {
    m_stream.seekg(count, std::ios_base::cur);
    return m_stream.tellg();
  }

  void close() override {}

private:
  std::string m_value;
  std::string m_content_type;
  detail::imemstream<char> m_stream;
};

class file_input_stream : public iinput_stream {
public:
  virtual ~file_input_stream() = default;
};

} // namespace detail
} // namespace webview

#endif // WEBVIEW_DETAIL_STREAM_HH
