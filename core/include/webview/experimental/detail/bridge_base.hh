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

#ifndef WEBVIEW_DETAIL_BRIDGE_BASE_HH
#define WEBVIEW_DETAIL_BRIDGE_BASE_HH

#include "../../detail/json.hh"
#include "../../macros.h"
#include "iscript_evaluator.hh"
#include "promise.hh"
#include "scripts.hh"
#include "user_content_manager_base.hh"

#include <functional>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace webview {

using binding_promise = detail::promise<std::string>;

class binding_arg {
public:
  binding_arg(binding_promise promise, void *user_data)
      : m_promise{std::move(promise)}, m_user_data{user_data} {}

  WEBVIEW_NODISCARD binding_promise get_promise() {
    return std::move(m_promise);
  }

  void *get_user_data() const { return m_user_data; }

private:
  binding_promise m_promise;
  void *m_user_data{};
};

using binding0_fn = std::function<void()>;
using binding1_fn = std::function<void(binding_arg &arg)>;

class ibridge {
public:
  virtual ~ibridge() = default;

  virtual void bind(const std::string &name, binding0_fn handler,
                    void *user_data = nullptr) = 0;
  virtual void bind(const std::string &name, binding1_fn handler,
                    void *user_data = nullptr) = 0;
  virtual void unbind(const std::string &name) = 0;
  virtual void unbind(const std::string &name,
                      std::function<void(void *user_data)> deleter) = 0;
};

using bridge_ptr = std::shared_ptr<ibridge>;

namespace detail {

class bridge_base : public ibridge {
  class mapping {
  public:
    explicit mapping(binding1_fn handler, void *user_data)
        : m_handler{std::move(handler)}, m_user_data{user_data} {}

    void invoke(binding_promise promise) {
      binding_arg arg{std::move(promise), m_user_data};
      m_handler(arg);
    }

    void *get_user_data() const { return m_user_data; }

  private:
    binding1_fn m_handler;
    void *m_user_data{};
  };

public:
  explicit bridge_base(user_content_manager_ptr user_content,
                       iscript_evaluator *script_evaluator)
      : m_user_content{user_content}, m_script_evaluator{script_evaluator} {

    m_user_content->events().message_received.bind(
        [=](iuser_content_manager * /*sender*/, const std::string &payload) {
          handle_received_message(payload);
          return true;
        },
        detail::get_internal_signal_priority());
  }

  virtual ~bridge_base() = default;

  void bind(const std::string &name, binding0_fn handler,
            void *user_data) override {
    bind(
        name,
        [=](binding_arg &arg) {
          handler();
          arg.get_promise().resolve();
        },
        user_data);
  }

  void bind(const std::string &name, binding1_fn handler,
            void *user_data) override {
    // NOLINTNEXTLINE(readability-container-contains): contains() requires C++20
    if (m_mappings.count(name) > 0) {
      //return error_info{WEBVIEW_ERROR_DUPLICATE};
      return;
    }
    m_mappings.emplace(name, mapping{std::move(handler), user_data});
    replace_bind_script();
    // Notify that a binding was created if the init script has already
    // set things up.
    m_script_evaluator->eval(scripts::create_call_on_bind_script(name));
  }

  void unbind(const std::string &name) override { unbind(name, {}); }

  void unbind(const std::string &name,
              std::function<void(void *user_data)> deleter) override {
    auto found{m_mappings.find(name)};
    if (found == m_mappings.end()) {
      //return error_info{WEBVIEW_ERROR_NOT_FOUND};
      return;
    }
    if (deleter) {
      deleter(found->second.get_user_data());
    }
    m_mappings.erase(found);
    replace_bind_script();
    // Notify that a binding was created if the init script has already
    // set things up.
    m_script_evaluator->eval(scripts::create_call_on_unbind_script(name));
  }

protected:
  void add_init_script(const std::string &post_fn) {
    m_user_content->add_script(scripts::create_init_script(post_fn),
                               user_script_injection_time::start);
  }

private:
  void replace_bind_script() {
    std::vector<std::string> names;
    names.reserve(m_mappings.size());
    for (const auto &mapping : m_mappings) {
      names.push_back(mapping.first);
    }
    auto script{scripts::create_bind_script(names)};
    if (m_bind_script) {
      // TOOD* make sure injection time is correct upon replacement
      m_bind_script = m_user_content->replace_script(m_bind_script, script);
    } else {
      const auto where{user_script_injection_time::start};
      m_bind_script = m_user_content->add_script(std::move(script), where);
    }
  }

  void handle_received_message(const std::string &payload) {
    auto id{json_parse(payload, "id", 0)};
    auto name{json_parse(payload, "method", 0)};
    auto params{json_parse(payload, "params", 0)};
    auto found{m_mappings.find(name)};
    if (found == m_mappings.end()) {
      m_script_evaluator->eval(scripts::create_reply_script(id, {}, false));
      return;
    }
    found->second.invoke(
        binding_promise{[=](std::string value) {
                          m_script_evaluator->eval(
                              scripts::create_reply_script(id, value, true));
                        },
                        [=](std::string value) {
                          m_script_evaluator->eval(
                              scripts::create_reply_script(id, value, false));
                        }});
  }

  user_content_manager_ptr m_user_content;
  iscript_evaluator *m_script_evaluator;
  std::unordered_map<std::string, mapping> m_mappings;
  user_script_ptr m_bind_script;
  std::list<user_script_ptr> m_user_scripts;
};

} // namespace detail
} // namespace webview

#endif // WEBVIEW_DETAIL_BRIDGE_BASE_HH
