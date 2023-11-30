# UI Framework Integration Example for Cocoa

This example is provided as a proof of concept. Since the underlying browser engine used in the webview library is WebKit, you may consider using it directly instead.

Key points:

* Required entitlements:
  * `com.apple.security.network.client`
* `WebviewBridge.mm` needs ARC turned off (`-fno-objc-arc`).
