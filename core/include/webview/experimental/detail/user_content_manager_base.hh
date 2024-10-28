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

#ifndef WEBVIEW_DETAIL_USER_CONTENT_MANAGER_BASE_HH
#define WEBVIEW_DETAIL_USER_CONTENT_MANAGER_BASE_HH

#include "../../detail/signal.hh"
#include "user_script_base.hh"

#include <memory>
#include <string>

namespace webview {

class iuser_content_manager;

class user_content_manager_events {
public:
  detail::signal<bool(iuser_content_manager *sender, std::string payload)>
      message_received;
};

class iuser_content_manager {
public:
  virtual ~iuser_content_manager() = default;

  virtual user_content_manager_events &events() = 0;

  virtual user_script_ptr add_script(const std::string &code,
                                     user_script_injection_time where) = 0;
  virtual user_script_ptr add_script(std::string &&code,
                                     user_script_injection_time where) = 0;
  virtual user_script_ptr replace_script(user_script_ptr old_script,
                                         const std::string &new_code) = 0;
  virtual void remove_script(user_script_ptr script) = 0;
  virtual void remove_all_scripts() = 0;
};

using user_content_manager_ptr = std::shared_ptr<iuser_content_manager>;

namespace detail {

class user_content_manager_base : public iuser_content_manager {
public:
  virtual ~user_content_manager_base() = default;
};

} // namespace detail
} // namespace webview

#endif // WEBVIEW_DETAIL_USER_CONTENT_MANAGER_BASE_HH
