//
//  ViewController.h
//  Cocoa Example
//
//  Created by Steffen on 2023/08/05.
//

#import <Cocoa/Cocoa.h>

@interface ViewController : NSViewController

@property (weak) IBOutlet NSTextField *locationTextField;
@property (weak) IBOutlet NSButton *goButton;
@property (weak) IBOutlet NSView *webContainer;
@property (weak) IBOutlet NSTextField *counterTextField;

@end

