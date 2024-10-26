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

#ifndef WEBVIEW_DETAIL_SCRIPTS_HH
#define WEBVIEW_DETAIL_SCRIPTS_HH

#include "../../detail/json.hh"

#include <string>

namespace webview {
namespace detail {
namespace scripts {

inline std::string create_call_on_bind_script(const std::string &name) {
  return "if (window.__webview__) {\n\
window.__webview__.onBind(" +
         json_escape(name) + ")\n\
}";
}

inline std::string create_reply_script(const std::string& id, const std::string &value, bool resolved) {
    std::string script;
    script += "window.__webview__.onReply(";
    script += json_escape(id);
    script += ", ";
    script += resolved ? "0" : "1";
    script += ", ";
    script += value.empty() ? "undefined" : json_escape(value);
    script += ")";
    return script;
}

inline std::string create_init_script(const std::string &post_fn) {
  auto js = std::string{} + "(function() {\n\
  'use strict';\n\
  function generateId() {\n\
    var crypto = window.crypto || window.msCrypto;\n\
    var bytes = new Uint8Array(16);\n\
    crypto.getRandomValues(bytes);\n\
    return Array.prototype.slice.call(bytes).map(function(n) {\n\
      var s = n.toString(16);\n\
      return ((s.length % 2) == 1 ? '0' : '') + s;\n\
    }).join('');\n\
  }\n\
  var Webview = (function() {\n\
    var _promises = {};\n\
    function Webview_() {}\n\
    Webview_.prototype.post = function(message) {\n\
      return (" +
            post_fn + ")(message);\n\
    };\n\
    Webview_.prototype.call = function(method) {\n\
      var _id = generateId();\n\
      var _params = Array.prototype.slice.call(arguments, 1);\n\
      var promise = new Promise(function(resolve, reject) {\n\
        _promises[_id] = { resolve, reject };\n\
      });\n\
      this.post(JSON.stringify({\n\
        id: _id,\n\
        method: method,\n\
        params: _params\n\
      }));\n\
      return promise;\n\
    };\n\
    Webview_.prototype.onReply = function(id, status, result) {\n\
      var promise = _promises[id];\n\
      if (result !== undefined) {\n\
        try {\n\
          result = JSON.parse(result);\n\
        } catch (e) {\n\
          promise.reject(new Error(\"Failed to parse binding result as JSON\"));\n\
          return;\n\
        }\n\
      }\n\
      if (status === 0) {\n\
        promise.resolve(result);\n\
      } else {\n\
        promise.reject(result);\n\
      }\n\
    };\n\
    Webview_.prototype.onBind = function(name) {\n\
      if (window.hasOwnProperty(name)) {\n\
        throw new Error('Property \"' + name + '\" already exists');\n\
      }\n\
      window[name] = (function() {\n\
        var params = [name].concat(Array.prototype.slice.call(arguments));\n\
        return Webview_.prototype.call.apply(this, params);\n\
      }).bind(this);\n\
    };\n\
    Webview_.prototype.onUnbind = function(name) {\n\
      if (!window.hasOwnProperty(name)) {\n\
        throw new Error('Property \"' + name + '\" does not exist');\n\
      }\n\
      delete window[name];\n\
    };\n\
    return Webview_;\n\
  })();\n\
  window.__webview__ = new Webview();\n\
})()";
  return js;
}

template <typename T> std::string create_bind_script(const T &names) {
  std::string js_names = "[";
  bool first = true;
  for (const auto &name : names) {
    if (first) {
      first = false;
    } else {
      js_names += ",";
    }
    js_names += json_escape(name);
  }
  js_names += "]";

  auto js = std::string{} + "(function() {\n\
  'use strict';\n\
  var methods = " +
            js_names + ";\n\
  methods.forEach(function(name) {\n\
    window.__webview__.onBind(name);\n\
  });\n\
})()";
  return js;
}

} // namespace scripts
} // namespace detail
} // namespace webview

#endif // WEBVIEW_DETAIL_SCRIPTS_HH
