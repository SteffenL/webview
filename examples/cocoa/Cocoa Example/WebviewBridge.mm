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
    _webview->navigate(std::string{url.UTF8String, [url lengthOfBytesUsingEncoding:NSUTF8StringEncoding]});
}

- (void)bindWithName:(NSString *)name block:(NSString *(^)(NSString * req))block {
    const std::string name_{name.UTF8String, [name lengthOfBytesUsingEncoding:NSUTF8StringEncoding]};
    id retainedBlock{(id)_Block_copy(block)}; //objc_retainBlock()
    _webview->bind(name_, [=](const std::string& req_) -> std::string {
        if (!retainedBlock) {
            return {};
        }
        NSString *req{[NSString stringWithUTF8String:req_.c_str()]};
        auto typedRetainedBlock = (NSString *(^)(NSString * req))retainedBlock;
        NSString *res{typedRetainedBlock(req)};
        //[retainedBlock release]; // causes crash the 2nd time around
        std::string res_{res.UTF8String, [res lengthOfBytesUsingEncoding:NSUTF8StringEncoding]};
        return res_;
    });
}

@end
