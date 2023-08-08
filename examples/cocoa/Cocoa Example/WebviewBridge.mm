//
//  WebviewBridge.mm
//  Cocoa Example
//
//  Created by Steffen on 2023/08/08.
//

#import "WebviewBridge.h"
#include "webview.h"

@implementation WebviewBridge

std::unique_ptr<webview::webview> _webview;

- (instancetype)initWithDebug:(BOOL)debug window:(NSWindow *)window {
    if ((self = [WebviewBridge alloc])) {
        _webview = std::unique_ptr<webview::webview>(new webview::webview{!!debug, (__bridge void *)window});
    }
    return self;
}

- (NSView *)widget {
    return (__bridge NSView *)_webview->widget();
}

- (void)setHTML:(NSString *)html {
    _webview->set_html(std::string{html.UTF8String, [html lengthOfBytesUsingEncoding:NSUTF8StringEncoding]});
}

- (void)navigate:(NSString *)url {
    
}

- (void)bindWithName:(NSString *)name block:(NSString *(^)(NSString * req))block {
    const std::string name_{name.UTF8String, [name lengthOfBytesUsingEncoding:NSUTF8StringEncoding]};
    _webview->bind(name_, [=](const std::string& req_) -> std::string {
        if (block) {
            NSString *req{[NSString stringWithUTF8String:req_.c_str()]};
            NSString *res{block(req)};
            std::string res_{res.UTF8String, [res lengthOfBytesUsingEncoding:NSUTF8StringEncoding]};
            return res_;
        }
        return {};
    });
}

@end
