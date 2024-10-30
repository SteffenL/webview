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

#include "../../detail/utility/string.hh"
#include "memstream.hh"

#include <cstdint>
#include <fstream>
#include <memory>
#include <string>

namespace webview {
namespace detail {

class iinput_stream {
public:
  virtual ~iinput_stream() = default;

  virtual bool good() const = 0;
  virtual bool eof() const = 0;
  virtual intptr_t tell() = 0;
  virtual intptr_t read(void *buffer, intptr_t count) = 0;
  virtual intptr_t seek(intptr_t count) = 0;
  virtual void close() = 0;
};

using input_stream_ptr = std::unique_ptr<iinput_stream>;

class string_input_stream : public iinput_stream {
public:
  string_input_stream(std::string value)
      : m_value{std::move(value)}, m_stream{m_value.data(), m_value.size()} {}

  bool good() const override { return m_stream.good(); }
  bool eof() const override { return m_stream.eof(); }
  intptr_t tell() override { return m_stream.tellg(); }

  intptr_t read(void *buffer, intptr_t count) override {
    m_stream.read(static_cast<char *>(buffer), count);
    return m_stream.gcount();
  }

  intptr_t seek(intptr_t count) override {
    m_stream.seekg(count, std::ios_base::cur);
    return m_stream.tellg();
  }

  void close() override {
    // Do nothing
  }

private:
  std::string m_value;
  std::string m_content_type;
  imemstream<char> m_stream;
};

inline input_stream_ptr make_string_input_stream(std::string value) {
  return input_stream_ptr{new string_input_stream(std::move(value))};
}

class file_input_stream : public iinput_stream {
public:
  virtual ~file_input_stream() = default;

  bool good() const override { return m_stream.good(); }
  bool eof() const override { return m_stream.eof(); }
  intptr_t tell() override { return m_stream.tellg(); }

  file_input_stream(const std::string &file_path)
      : m_stream{make_file_path_string(file_path)} {}

  intptr_t read(void *buffer, intptr_t count) override {
    m_stream.read(static_cast<char *>(buffer), count);
    auto a = m_stream.gcount();
    return a;
  }

  intptr_t seek(intptr_t count) override {
    m_stream.seekg(count, std::ios_base::cur);
    return m_stream.tellg();
  }

  void close() override { m_stream.close(); }

private:
#if defined(_WIN32)
  static std::wstring make_file_path_string(const std::string &input) {
    return widen_string(input);
  }
#else
  static std::string make_file_path_string(const std::string &input) {
    return input;
  }
#endif

  mutable std::ifstream m_stream;
};

inline input_stream_ptr make_file_input_stream(const std::string &file_path) {
  return input_stream_ptr{new file_input_stream(file_path)};
}

} // namespace detail
} // namespace webview

#endif // WEBVIEW_DETAIL_STREAM_HH
