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

#ifndef WEBVIEW_DETAIL_THREAD_HH
#define WEBVIEW_DETAIL_THREAD_HH

#include <thread>

namespace webview {
namespace detail {

class thread {
public:
  thread() = default;

  template <typename F, typename... Args>
  thread(F &&fn, Args &&...args)
      : m_thread{std::forward<F>(fn), std::forward<Args>(args)...} {}

  thread(const thread &) = delete;
  thread &operator=(const thread &) = delete;

  thread(thread &&other) noexcept { *this = std::move(other); }

  thread &operator=(thread &&other) noexcept {
    if (this != &other) {
      if (m_thread.joinable()) {
        m_thread.join();
      }
      m_thread = std::move(other.m_thread);
    }
    return *this;
  }

  ~thread() {
    if (m_thread.joinable()) {
      m_thread.join();
    }
  }

private:
  std::thread m_thread;
};

} // namespace detail
} // namespace webview

#endif // WEBVIEW_DETAIL_THREAD_HH
