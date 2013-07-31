#import <Cocoa/Cocoa.h>

@interface O0GameView : NSView
- (id) initWithFrame:(NSRect)frame
           localPort:(int)port;

- (id) initWithFrame:(NSRect)frame
          remoteHost:(const char*)host
          remotePort:(int)port;
@end
