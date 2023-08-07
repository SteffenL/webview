//
//  ViewController.m
//  Cocoa Example
//
//  Created by Steffen on 2023/08/05.
//

#import "ViewController.h"
#include "webview.h"

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // Do any additional setup after loading the view.
}


- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // Update the view, if already loaded.
}

- (IBAction)goButtonPressed:(NSButton *)sender {
    auto *container{(__bridge_retained void *) _webContainer};
    webview::webview w{false, container};
    auto *url{_locationTextField.stringValue};
    w.navigate(std::string{url.UTF8String, [url lengthOfBytesUsingEncoding:NSUTF8StringEncoding]});
}


@end
