#ifndef TEST_H
#define TEST_H

#include <cassert>
#include <functional>
#include <map>
#include <memory>
#include <string>

// Utility macro for stringifying a macro argument.
#define TEST_STRINGIFY(x) #x

#define TEST(name)                                                             \
  void test_fn_##name();                                                       \
  static const test::test_registration test_reg_##name{TEST_STRINGIFY(name),   \
                                                       test_fn_##name};        \
  void test_fn_##name()

namespace test {

using test_fn = std::function<void()>;

class test_storage {
public:
  static test_storage &instance() noexcept;
  void add(std::string &&name, test_fn &&fn) noexcept;
  const std::map<std::string, test_fn> &get_items() const noexcept;

private:
  static std::unique_ptr<test_storage> m_instance;
  std::map<std::string, test_fn> m_items;
};

class test_registration {
public:
  test_registration(std::string &&name, test_fn &&fn) noexcept;
};

} // namespace test

#endif /* TEST_H */
