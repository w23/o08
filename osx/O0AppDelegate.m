#import "O0GameView.h"
#import "O0AppDelegate.h"

@implementation O0AppDelegate
- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
  // Insert code here to initialize your application
  [self.window.contentView addSubview:[[O0GameView alloc] initWithFrame:[self.window.contentView bounds] localPort:7075]];
}
@end
