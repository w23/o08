#import "O0GameView.h"
#import <kapusha/sys/osx/KPView.h>
#import "Game.h"

@interface O0GameView ()
- (id) initWithFrame:(NSRect)frame viewport:(kapusha::IViewport*)viewport;
@end

@implementation O0GameView
- (id) initWithFrame:(NSRect)frame viewport:(kapusha::IViewport*)viewport
{
  self = [super initWithFrame:frame];
  if (self) {
    self.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    KPView *kpv = [[KPView alloc] initWithFrame:frame WithViewport:viewport];
    kpv.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    [self addSubview:kpv];
  }
  return self;
}

- (id) initWithFrame:(NSRect)frame
           localPort:(int)port
{
  return [self initWithFrame:frame
                    viewport:new Game(port)];
}

- (id) initWithFrame:(NSRect)frame
          remoteHost:(const char*)host
          remotePort:(int)port
{
  return [self initWithFrame:frame
                    viewport:new Game(host, port)];
}
@end
