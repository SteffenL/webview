//
//  WebviewBridge.h
//  Cocoa Example
//

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

@interface WebviewBridge : NSObject

- (instancetype)initWithDebug:(BOOL)debug window:(NSWindow *)window;
- (NSView *)widget;
- (void)setHTML:(NSString *)html;
- (void)navigate:(NSString *)url;
- (void)bindWithName:(NSString *)name block:(NSString *(^)(NSString * req))block;

@end

NS_ASSUME_NONNULL_END
