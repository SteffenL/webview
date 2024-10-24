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

#ifndef WEBVIEW_DETAIL_BROWSER_BASE_HH
#define WEBVIEW_DETAIL_BROWSER_BASE_HH

#include "../../detail/signal.hh"
#include "../../types.hh"

#include <memory>
#include <string>

namespace webview {

class browser_events {
public:
  detail::signal<void()> ready;
};

class ibrowser {
public:
  virtual ~ibrowser() = default;

  virtual browser_events &events() = 0;
  virtual void *get_native_handle() const = 0;
  virtual void *get_native_embeddable() = 0;

  virtual void navigate(const std::string &url) = 0;
  virtual void set_html(const std::string &html) = 0;
};

using browser_ptr = std::shared_ptr<ibrowser>;

namespace detail {

class browser_base : public ibrowser {
public:
  virtual ~browser_base() = default;
};

} // namespace detail
} // namespace webview

#endif // WEBVIEW_DETAIL_BROWSER_BASE_HH
