#include "test.hpp"

namespace test {

test_storage &test_storage::instance() noexcept {
  static test_storage instance;
  return instance;
}

void test_storage::add(std::string &&name, test_fn &&fn) noexcept {
  m_items.emplace(std::move(name), std::move(fn));
}

const std::map<std::string, test_fn> &test_storage::get_items() const noexcept {
  return m_items;
}

test_registration::test_registration(std::string &&name,
                                     test_fn &&fn) noexcept {
  test_storage::instance().add(std::move(name), std::move(fn));
}

} // namespace test
