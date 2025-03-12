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

#ifndef WEBVIEW_PLATFORM_WINDOWS_COM_STRING_HH
#define WEBVIEW_PLATFORM_WINDOWS_COM_STRING_HH

#include "../../../macros.h"

#if defined(WEBVIEW_PLATFORM_WINDOWS)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <cwchar>
#include <utility>

#include <objbase.h>

#ifdef _MSC_VER
#pragma comment(lib, "ole32.lib")
#endif

namespace webview {
namespace detail {

class com_string {
public:
  com_string() = default;

  com_string(LPCWSTR s) {
    auto length{::wcslen(s)};
    if (auto mem{copy_mem(s, length)}) {
      m_ptr = mem;
      m_length = length;
    }
  }

  com_string(const com_string &other) { *this = other; }

  com_string &operator=(const com_string &other) {
    if (this == &other) {
      return *this;
    }
    m_ptr = other.copy();
    m_length = other.m_length;
    return *this;
  }

  com_string(com_string &&other) noexcept { *this = std::move(other); }

  com_string &operator=(com_string &&other) noexcept {
    if (this == &other) {
      return *this;
    }
    m_ptr = other.m_ptr;
    other.m_ptr = nullptr;
    m_length = other.m_length;
    other.m_length = 0;
    return *this;
  }

  ~com_string() {
    if (m_ptr) {
      ::CoTaskMemFree(m_ptr);
    }
  }

  LPWSTR copy() const { return copy_mem(m_ptr, m_length); }

private:
  com_string(LPWSTR s, SIZE_T length) : m_ptr{s}, m_length{length} {}

  static LPWSTR copy_mem(LPCWSTR from, SIZE_T length) {
    if (!from) {
      return nullptr;
    }
    auto length_bytes{(length + 1) * sizeof(from[0])};
    if (auto mem{::CoTaskMemAlloc(length_bytes)}) {
      ::memcpy(mem, from, length_bytes);
      return static_cast<LPWSTR>(mem);
    }
    return nullptr;
  }

  LPWSTR m_ptr{};
  SIZE_T m_length{};
};

} // namespace detail
} // namespace webview

#endif
#endif // WEBVIEW_PLATFORM_WINDOWS_COM_STRING_HH
