#import "O0GameView.h"
#import "O0AppDelegate.h"

@interface O0AppDelegate ()
@property (weak) IBOutlet NSView *configView;
@property (weak) IBOutlet NSTextField *remoteIP;
@property (weak) IBOutlet NSTextField *remotePort;
@property (weak) IBOutlet NSTextField *localPort;
- (IBAction)actionConnect:(id)sender;
- (IBAction)actionCreate:(id)sender;
@end

@implementation O0AppDelegate
- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
}

- (IBAction)actionConnect:(id)sender {
  self.configView.hidden = YES;
  O0GameView *game = [[O0GameView alloc] initWithFrame:[self.window.contentView bounds]
                                            remoteHost:[self.remoteIP.stringValue UTF8String]
                                            remotePort:(int)self.remotePort.integerValue];
  [self.window.contentView addSubview:game];
  [self.window makeFirstResponder:game];
}

- (IBAction)actionCreate:(id)sender {
  self.configView.hidden = YES;
  O0GameView *game = [[O0GameView alloc] initWithFrame:[self.window.contentView bounds]
                                             localPort:(int)self.localPort.integerValue];
  [self.window.contentView addSubview:game];
  [self.window makeFirstResponder:game];
}
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
  return YES;
}
@end
