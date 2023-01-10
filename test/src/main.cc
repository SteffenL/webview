#include "test.hh"

#include <atomic>
#include <iostream>
#include <thread>

void run_with_timeout(test::test_fn fn, int timeout_ms) {
  std::atomic_flag flag_running = ATOMIC_FLAG_INIT;
  flag_running.test_and_set();
  std::thread timeout_thread([&]() {
    for (int i = 0; i < timeout_ms / 100; i++) {
      if (!flag_running.test_and_set()) {
        return;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << "Exiting due to a timeout." << std::endl;
    exit(1);
  });
  fn();
  flag_running.clear();
  timeout_thread.join();
}

int main(int argc, char *argv[]) {
  const auto &all_tests = test::test_storage::instance().get_items();
  // Without arguments run all tests, one-by-one by forking itself.
  // With a single argument - run the requested test
  if (argc == 1) {
    int failed = 0;
    for (const auto &test : all_tests) {
      std::cout << "TEST: " << test.first << std::endl;
      int status = system((std::string(argv[0]) + " " + test.first).c_str());
      if (status == 0) {
        std::cout << "  PASS " << std::endl;
      } else {
        std::cout << "  FAIL: " << status << std::endl;
        failed = 1;
      }
    }
    return failed;
  }

  if (argc == 2) {
    auto it = all_tests.find(argv[1]);
    if (it != all_tests.end()) {
      run_with_timeout(it->second, 60000);
      return 0;
    }
  }
  std::cout << "USAGE: " << argv[0] << " [test name]" << std::endl;
  std::cout << "Tests: " << std::endl;
  for (const auto &test : all_tests) {
    std::cout << "  " << test.first << std::endl;
  }
  return 1;
}
