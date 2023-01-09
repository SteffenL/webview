#include "test.hpp"

namespace test {

std::unique_ptr<test_storage> test_storage::m_instance;

test_storage &test_storage::instance() noexcept {
  if (!m_instance) {
    m_instance = std::unique_ptr<test_storage>(new test_storage);
  }
  return *m_instance;
}

void test_storage::add(std::string &&name, test_fn &&fn) noexcept {
  m_items.emplace(std::move(name), std::move(fn));
}

const std::map<std::string, test_fn> &test_storage::get_items() const noexcept {
  return m_items;
}

test_registration::test_registration(std::string &&name,
                                     test_fn &&fn) noexcept {
  test_storage::instance().add(std::forward<decltype(name)>(name),
                               std::forward<decltype(fn)>(fn));
}

} // namespace test
