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

enum class signal_priority { lowest, low, medium, high, highest };

constexpr signal_priority get_default_signal_priority() noexcept {
  return signal_priority::medium;
}

namespace detail {

constexpr signal_priority get_internal_signal_priority() noexcept {
  using int_type = typename std::underlying_type<signal_priority>::type;
  return static_cast<signal_priority>(
      static_cast<int_type>(signal_priority::lowest) - 1);
}

template <typename T> class signal {
  using function_type = std::function<T>;
  using pointer_type = signal<T> *;
  using result_type = typename function_type::result_type;

  class handler_type {
  public:
    handler_type(function_type fn, signal_priority priority)
        : m_function{fn}, m_priority{priority} {}

    handler_type(pointer_type sig, signal_priority priority)
        : m_signal{sig}, m_priority{priority} {}

    template <typename R = result_type, typename... Args>
    typename std::enable_if<std::is_same<R, void>::value, bool>::type
    call(Args &&...args) const {
      if (m_function) {
        m_function(std::forward<Args>(args)...);
      } else if (m_signal) {
        return m_signal->emit(std::forward<Args>(args)...);
      }
      return false;
    }

    template <typename R = result_type, typename... Args>
    typename std::enable_if<std::is_same<R, bool>::value, bool>::type
    call(Args &&...args) const {
      if (m_function) {
        return m_function(std::forward<Args>(args)...);
      } else if (m_signal) {
        return m_signal->emit(std::forward<Args>(args)...);
      }
      return false;
    }

    bool operator<(const handler_type &other) const noexcept {
      using type = typename std::underlying_type<decltype(m_priority)>::type;
      return static_cast<type>(m_priority) >
             static_cast<type>(other.m_priority);
    }

  private:
    function_type m_function{};
    pointer_type m_signal{};
    signal_priority m_priority{};
  };

public:
  void bind(function_type handler,
            signal_priority priority = get_default_signal_priority()) {
    m_handlers.push_back(handler_type{handler, priority});
    sort_handlers();
  }

  void bind(signal<T> &fwd_signal,
            signal_priority priority = get_default_signal_priority()) {
    m_handlers.push_back(handler_type{&fwd_signal, priority});
    sort_handlers();
  }

  template <typename R, typename... Args>
  void unbind(std::function<R(Args...)> handler) {
    auto found{std::find(m_handlers.begin(), m_handlers.end(), handler)};
    if (found != m_handlers.end()) {
      m_handlers.erase(found);
    }
  }

  template <typename... Args> bool emit(Args &&...args) const {
    for (auto &handler : m_handlers) {
      if (handler.call(std::forward<Args>(args)...)) {
        return true;
      }
    }
    return false;
  }

private:
  void sort_handlers() {
    std::stable_sort(m_handlers.begin(), m_handlers.end());
  }

  std::vector<handler_type> m_handlers;
};

} // namespace detail
} // namespace webview

#endif // WEBVIEW_DETAIL_SIGNAL_HH
