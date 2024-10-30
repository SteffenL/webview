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

#ifndef WEBVIEW_DETAIL_MEMSTREAM_HH
#define WEBVIEW_DETAIL_MEMSTREAM_HH

#include "memstreambuf.hpp"

#include <istream>

namespace webview {
namespace detail {

template <typename Char, typename Traits = std::char_traits<Char>>
class imemstream : public std::basic_istream<Char, Traits> {
public:
  imemstream(const Char *start, const Char *end)
      : std::basic_istream<Char, Traits>{nullptr},
        m_buffer{cast(start), cast(end)} {
    this->set_rdbuf(&m_buffer);
  }

  imemstream(const Char *start, size_t length)
      : imemstream{start, start + length} {}

private:
  static constexpr Char *cast(const Char *p) noexcept {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    return const_cast<typename std::add_pointer<
        typename std::remove_const<Char>::type>::type>(p);
  }

  memstreambuf<Char, Traits> m_buffer;
};

} // namespace detail
} // namespace webview

#endif // WEBVIEW_DETAIL_ISCRIPT_EVALUATOR_HH
