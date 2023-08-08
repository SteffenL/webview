//
//  ViewController.m
//  Cocoa Example
//
//  Created by Steffen on 2023/08/05.
//

#import "ViewController.h"
#include "webview.h"

static constexpr const auto html =
    R"html(<button id="increment">Tap me</button>
<script>
  const [incrementElement] = document.querySelectorAll("#increment");
  document.addEventListener("DOMContentLoaded", () => {
    incrementElement.addEventListener("click", () => {
      window.increment();
    });
  });
</script>)html";

@implementation ViewController

std::unique_ptr<webview::webview> m_webview;

- (void)viewDidLoad {
    [super viewDidLoad];

    // Do any additional setup after loading the view.

    _locationTextField.stringValue = @"https://github.com/webview/webview";

    m_webview = std::unique_ptr<webview::webview>(new webview::webview{false, _webContainer.window});
    NSView *widget = (NSView *)m_webview->widget();
    widget.translatesAutoresizingMaskIntoConstraints = NO;
    [_webContainer addSubview:widget];
    NSDictionary *views = NSDictionaryOfVariableBindings(widget);
    [_webContainer addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"H:|[widget]|"
                                                                          options:0
                                                                          metrics:nil
                                                                            views:views]];
    [_webContainer addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|[widget]|"
                                                                          options:0
                                                                          metrics:nil
                                                                            views:views]];
    m_webview->set_html(html);
}

- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // Update the view, if already loaded.
}

- (IBAction)goButtonPressed:(NSButton *)sender {
    NSString *url = _locationTextField.stringValue;
    m_webview->navigate(std::string{url.UTF8String, [url lengthOfBytesUsingEncoding:NSUTF8StringEncoding]});
}


@end
