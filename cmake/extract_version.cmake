# Extracts the library's version number and prints it to stdout.

include("${CMAKE_CURRENT_LIST_DIR}/internal.cmake")
webview_extract_version()
message("${WEBVIEW_VERSION}")
