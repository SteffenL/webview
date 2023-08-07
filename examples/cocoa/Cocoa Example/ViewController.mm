//
//  ViewController.m
//  Cocoa Example
//
//  Created by Steffen on 2023/08/05.
//

#import "ViewController.h"
#include "webview.h"

@implementation ViewController

std::unique_ptr<webview::webview> m_webview;

- (void)viewDidLoad {
    [super viewDidLoad];

    // Do any additional setup after loading the view.

    //auto *container{(__bridge void *) _webContainer};
    NSWindow *window = _webContainer.window; //[[NSApplication sharedApplication] mainWindow];
    m_webview = std::unique_ptr<webview::webview>(new webview::webview{false, window});
    NSView *widget = (NSView *)m_webview->widget();
    widget.bounds = _webContainer.bounds;
    widget.autoresizesSubviews = YES;
    widget.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    [_webContainer addSubview:(NSView *)m_webview->widget()];
}


- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // Update the view, if already loaded.
}

- (IBAction)goButtonPressed:(NSButton *)sender {
    auto *url{_locationTextField.stringValue};
    m_webview->navigate(std::string{url.UTF8String, [url lengthOfBytesUsingEncoding:NSUTF8StringEncoding]});
}


@end
