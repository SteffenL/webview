#ifdef WEBVIEW_BUILDING
#error WEBVIEW_BUILDING must not be defined
#endif

#include "webview.h"
#include <iostream>

namespace options {

constexpr auto const yes = "yes"; // NOLINT(unused-const-variable)
constexpr auto const no = "no";   // NOLINT(unused-const-variable)

constexpr auto const libraryType =
#if defined(WEBVIEW_SHARED)
    "shared"
#elif defined(WEBVIEW_STATIC)
    "static"
#else
    "header-only"
#endif
    ;

constexpr auto const implIncluded =
#ifdef WEBVIEW_INCLUDE_IMPLEMENTATION
    yes
#else
    no
#endif
    ;

constexpr auto const implOptOut =
#ifdef WEBVIEW_HEADER
    yes
#else
    no
#endif
    ;

} // namespace options

int main() {
  webview_destroy(webview_create(0, nullptr));
  std::cout << "Type: " << options::libraryType << std::endl;
  std::cout << "Implementation included: " << options::implIncluded
            << std::endl;
  std::cout << "Implementation opt-out: " << options::implOptOut << std::endl;
  return 0;
}
