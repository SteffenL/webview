set -e

if [[ ! -d build ]]; then
  mkdir build
fi

c++ -c -g -O0 -std=c++11 -I. -DWEBVIEW_STATIC -o build/webview.o webview.cc
cc -c -g -O0 -std=c99 -I. -o build/bind.o examples/bind.c
c++ -g -O0 -framework WebKit -o build/bind build/webview.o build/bind.o

lldb --batch \
  -o 'breakpoint set --command "frame info" --auto-continue true --name "-[NSWindow initWithContentRect:styleMask:backing:defer:]"' \
  -o 'breakpoint set --command "frame info" --auto-continue true --name "-[NSWindow dealloc]"' \
  -o 'breakpoint set --command "frame info" --auto-continue true --name "-[WKWebView initWithFrame:configuration:]"' \
  -o 'breakpoint set --command "frame info" --auto-continue true --name "-[WKWebView dealloc]"' \
  -o 'breakpoint set --command "frame info" --auto-continue true --name "-[WKUserContentController dealloc]"' \
  -o 'breakpoint set --command "frame info" --auto-continue true --name webview::detail::cocoa_wkwebview_engine::app_delegate_dealloc' \
  -o 'breakpoint set --command "frame info" --auto-continue true --name webview::detail::cocoa_wkwebview_engine::window_delegate_dealloc' \
  -o 'breakpoint set --command "frame info" --auto-continue true --name webview::detail::cocoa_wkwebview_engine::webkit_ui_delegate_dealloc' \
  -o 'breakpoint set --command "frame info" --auto-continue true --name webview::detail::cocoa_wkwebview_engine::script_message_handler_dealloc' \
  -o run \
  ./build/bind

# Commented out because this looks fine.
#  -o 'breakpoint set --command "frame info" --auto-continue true --name webview::detail::user_script::impl::impl' \
#  -o 'breakpoint set --command "frame info" --auto-continue true --name webview::detail::user_script::impl::~impl' \

# Why is -[NSWindow initWithContentRect:styleMask:backing:defer:] invoked twice but -[NSWindow dealloc] only once? Use command "thread backtrace" instead of "frame info" for more info.
# "-[WKUserContentController dealloc] invoked when closing window, but not when calling `webview_destroy` first.
# Same with `script_message_handler_dealloc`.
