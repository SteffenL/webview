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

    auto *container{(__bridge void *) _webContainer};
    //m_webview = std::unique_ptr<webview::webview>(new webview::webview{false, container});
    webview::webview w{false, container};
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
