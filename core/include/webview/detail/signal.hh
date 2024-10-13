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

#ifndef WEBVIEW_DETAIL_SIGNAL_HH
#define WEBVIEW_DETAIL_SIGNAL_HH

#include <algorithm>
#include <functional>
#include <vector>

namespace webview {
namespace detail {

template <typename T> class signal {
public:
  void bind(std::function<T> handler) { m_handlers.push_back(handler); }

  void unbind(std::function<T> handler) {
    auto found{std::find(m_handlers.begin(), m_handlers.end(), handler)};
    if (found != m_handlers.end()) {
      m_handlers.erase(found);
    }
  }

  template <typename... Args> void emit(Args &&...args) {
    const auto handlers{m_handlers};
    for (const auto &handler : handlers) {
      handler(std::forward<Args>(args)...);
    }
  }

private:
  std::vector<std::function<T>> m_handlers;
};

} // namespace detail
} // namespace webview

#endif // WEBVIEW_DETAIL_SIGNAL_HH
