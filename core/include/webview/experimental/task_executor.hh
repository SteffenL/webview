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

#ifndef WEBVIEW_DETAIL_TASK_EXECUTOR_HH
#define WEBVIEW_DETAIL_TASK_EXECUTOR_HH

#include "detail/thread.hh"

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace webview {

class task_executor {
  using ulock_t = std::unique_lock<std::mutex>;
  using lock_t = std::lock_guard<std::mutex>;

public:
  using task_fn = std::function<void()>;

  task_executor(size_t thread_count = std::thread::hardware_concurrency()) {
    for (size_t i{}; i < thread_count; ++i) {
      m_worker_threads.emplace_back([this] {
        while (true) {
          task_fn task;
          {
            ulock_t lock{m_mutex};
            m_cv.wait(lock, [this] {
              return m_cancel || m_stop || !m_queue.empty();
            });
            if (m_cancel || (m_stop && m_queue.empty())) {
              return;
            }
            if (m_queue.empty()) {
              continue;
            }
            task = std::move(m_queue.front());
            m_queue.pop();
          }
          task();
        }
      });
    }
  }

  ~task_executor() {
    {
      lock_t lock{m_mutex};
      m_stop = true;
    }
    m_cv.notify_all();
  }

  void cancel() {
    {
      lock_t lock{m_mutex};
      m_cancel = true;
    }
    m_cv.notify_all();
  }

  template <typename F, typename... Args> void put(F &&fn, Args &&...args) {
    {
      lock_t lock{m_mutex};
      if (m_stop) {
        return;
      }
      m_queue.push(std::bind(std::forward<F>(fn), std::forward<Args>(args)...));
    }
    m_cv.notify_one();
  }

private:
  std::mutex m_mutex;
  std::condition_variable m_cv;
  bool m_stop{};
  bool m_cancel{};
  std::queue<task_fn> m_queue;
  std::vector<detail::thread> m_worker_threads;
};

} // namespace webview

#endif // WEBVIEW_DETAIL_TASK_EXECUTOR_HH
