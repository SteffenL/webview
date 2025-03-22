//
//  WebviewBridge.h
//  Cocoa Example
//

#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface WebviewBridge : NSObject

- (instancetype)initWithDebug:(BOOL)debug window:(NSResponder *)window;
- (NSView *)widget;
- (void)setHTML:(NSString *)html;
- (void)navigate:(NSString *)url;
- (void)bindWithName:(NSString *)name
               block:(NSString * (^)(NSString *req))block;

@end

NS_ASSUME_NONNULL_END
