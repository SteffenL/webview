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

#ifndef WEBVIEW_DETAIL_PROMISE_HH
#define WEBVIEW_DETAIL_PROMISE_HH

#include <functional>
#include <memory>

namespace webview {
namespace detail {

template <typename T, typename E = T> class promise {
public:
  using resolve_fn = std::function<void(T value)>;
  using reject_fn = std::function<void(E value)>;

private:
  class shared_state {
  public:
    shared_state(resolve_fn resolve_, reject_fn reject_)
        : resolve{resolve_}, reject{reject_} {}

    resolve_fn resolve;
    reject_fn reject;
    bool invoked{};
  };

public:
  promise(resolve_fn resolve, reject_fn reject)
      : m_shared_state{new shared_state{resolve, reject}} {}

  // Must be copyable to be used with std::function
  promise(const promise &) = default;
  promise &operator=(const promise &) = default;
  promise(promise &&) = default;
  promise &operator=(promise &&) = default;
  ~promise() = default;

  void resolve() {
    if (!m_shared_state->invoked) {
      m_shared_state->invoked = true;
      m_shared_state->resolve({});
    }
  }

  void resolve(T value) {
    if (!m_shared_state->invoked) {
      m_shared_state->invoked = true;
      m_shared_state->resolve(std::move(value));
    }
  }

  void reject() {
    if (!m_shared_state->invoked) {
      m_shared_state->invoked = true;
      m_shared_state->reject({});
    }
  }

  void reject(E value) {
    if (!m_shared_state->invoked) {
      m_shared_state->invoked = true;
      m_shared_state->reject(std::move(value));
    }
  }

private:
  // Share state between copies because std::function requires this class to
  // be copyable.
  std::shared_ptr<shared_state> m_shared_state;
};

} // namespace detail
} // namespace webview

#endif // WEBVIEW_DETAIL_PROMISE_HH
