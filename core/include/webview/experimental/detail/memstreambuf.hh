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

#ifndef WEBVIEW_DETAIL_MEMSTREAMBUF_HH
#define WEBVIEW_DETAIL_MEMSTREAMBUF_HH

#include <streambuf>

namespace webview {
namespace detail {

template <typename Char, typename Traits = std::char_traits<Char>>
class memstreambuf : public std::basic_streambuf<Char, Traits> {
public:
  memstreambuf(Char *start, Char *end) { this->setg(start, start, end); }

  std::streambuf::pos_type seekoff(std::streambuf::off_type offset,
                                   std::ios_base::seekdir dir,
                                   std::ios_base::openmode mode) override {
    if ((mode & std::ios_base::in) == 0) {
      // Not supported
      return std::streambuf::pos_type{std::streambuf::off_type{-1}};
    }
    if (dir == std::ios_base::beg) {
      return seekpos(offset, mode);
    }
    if (dir == std::ios_base::end) {
      auto pos{static_cast<std::streambuf::pos_type>(
          (this->egptr() - this->eback()) + offset)};
      return seekpos(pos, mode);
    }
    if (dir == std::ios_base::cur) {
      auto pos{static_cast<std::streambuf::pos_type>(
          (this->gptr() - this->eback()) + offset)};
      return seekpos(pos, mode);
    }
    // Should never happen
    return std::streambuf::pos_type{std::streambuf::off_type{-1}};
  }

  std::streambuf::pos_type seekpos(std::streambuf::pos_type pos,
                                   std::ios_base::openmode mode) override {
    if ((mode & std::ios_base::in) == 0) {
      // Not supported
      return std::streambuf::pos_type{std::streambuf::off_type{-1}};
    }
    if (this->eback() + pos < this->eback() ||
        this->eback() + pos >= this->egptr()) {
      // Out of range
      return std::streambuf::pos_type{std::streambuf::off_type{-1}};
    }
    this->setg(this->eback(), this->eback() + pos, this->egptr());
    return static_cast<std::streambuf::pos_type>(this->gptr() - this->eback());
  }
};

} // namespace detail
} // namespace webview

#endif // WEBVIEW_DETAIL_ISCRIPT_EVALUATOR_HH
