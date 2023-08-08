//
//  ViewController.mm
//  Cocoa Example
//

#import "ViewController.h"
#import "WebviewBridge.h"

static constexpr const auto html =
    @R"html(<button id="increment">Tap me</button>
<script>
  const [incrementElement] = document.querySelectorAll("#increment");
  document.addEventListener("DOMContentLoaded", () => {
    incrementElement.addEventListener("click", () => {
      window.increment();
    });
  });
</script>)html";

@implementation ViewController {
    WebviewBridge *_bridge;
    NSNumber *_counter;
}

- (void)viewDidLoad {
    [super viewDidLoad];

    // Do any additional setup after loading the view.

    _locationTextField.stringValue = @"https://github.com/webview/webview";
    _bridge = [[WebviewBridge alloc] initWithDebug:NO window:_webContainer.window];
    _counter = @0;

    NSView *widget = _bridge.widget;
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

    __block typeof(self->_counter) counter = _counter;
    __block typeof(self->_counterTextField) counterTextField = _counterTextField;
    [_bridge bindWithName: @"increment" block:^(NSString * req) {
        counter = [NSNumber numberWithInt:counter.intValue + 1];
        counterTextField.stringValue = counter.stringValue;
        return @"";
    }];

    [_bridge setHTML:html];

}

- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // Update the view, if already loaded.
}

- (IBAction)goButtonPressed:(NSButton *)sender {
    [_bridge navigate:_locationTextField.stringValue];
}


@end
