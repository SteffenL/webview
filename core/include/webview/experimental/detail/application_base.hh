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

#ifndef WEBVIEW_DETAIL_APPLICATION_BASE_HH
#define WEBVIEW_DETAIL_APPLICATION_BASE_HH

#include "../../detail/signal.hh"
#include "../../types.hh"

#include <memory>

namespace webview {

class iapplication;

class application_events {
public:
  detail::signal<void(iapplication *sender)> ready;
};

class iapplication {
public:
  virtual ~iapplication() = default;

  virtual void run() = 0;
  virtual void terminate() = 0;
  virtual void dispatch(dispatch_fn_t f) = 0;
  virtual application_events &events() = 0;
};

using application_ptr = std::shared_ptr<iapplication>;

namespace detail {

class application_base : public iapplication {
public:
  virtual ~application_base() = default;
};

inline iapplication *&current_application() {
  static iapplication *instance{};
  return instance;
}

inline void set_current_application(iapplication *instance) {
  current_application() = instance;
}

} // namespace detail

inline iapplication &current_application() {
  return *detail::current_application();
}

} // namespace webview

#endif // WEBVIEW_DETAIL_APPLICATION_BASE_HH
