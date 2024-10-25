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

#ifndef WEBVIEW_DETAIL_USER_SCRIPT_BASE_HH
#define WEBVIEW_DETAIL_USER_SCRIPT_BASE_HH

#include <memory>
#include <string>

namespace webview {

class iuser_script;

using user_script_ptr = std::shared_ptr<iuser_script>;

class iuser_script {
public:
  virtual ~iuser_script() = default;
  virtual void *get_native_handle() const = 0;
  virtual const std::string &get_code() const = 0;
  virtual bool equals(user_script_ptr script) const = 0;
};

using user_script_ptr = std::shared_ptr<iuser_script>;

namespace detail {

class user_script_base : public iuser_script {
public:
  virtual ~user_script_base() = default;

  bool equals(user_script_ptr script) const override {
    return get_native_handle() == script->get_native_handle();
  }
};

} // namespace detail
} // namespace webview

#endif // WEBVIEW_DETAIL_USER_SCRIPT_BASE_HH
