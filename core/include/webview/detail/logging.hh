#ifndef WEBVIEW_DETAIL_LOGGING_HH
#define WEBVIEW_DETAIL_LOGGING_HH

#include <chrono>
#include <ctime>
#include <iostream>
#include <string>
#include <thread>

namespace webview {
namespace detail {

inline std::string
format_time_point(const std::chrono::system_clock::time_point &tp) {
  std::time_t t{std::chrono::system_clock::to_time_t(tp)};
  std::string s{std::ctime(&t)};
  s.resize(s.size() - 1);
  return s;
}

template <typename Arg> void print_(Arg &&arg) {
  std::cout << std::forward<Arg>(arg);
}

template <typename Arg, typename... Args>
void print_(Arg &&arg, Args &&...args) {
  std::cout << std::forward<Arg>(arg);
  print_(std::forward<Args>(args)...);
}

template <typename... Args> void print(Args &&...args) {
  std::cout << "[" << format_time_point(std::chrono::system_clock::now())
            << "][" << std::this_thread::get_id() << "] ";
  print_(std::forward<Args>(args)...);
}

} // namespace detail
} // namespace webview

#endif // WEBVIEW_DETAIL_LOGGING_HH
