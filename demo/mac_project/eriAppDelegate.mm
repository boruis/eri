//
//  eriAppDelegate.m
//  eri
//
//  Created by exe on 10/16/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "eriAppDelegate.h"

#include "root.h"
#include "renderer.h"

#include "demo_app.h"

static DemoApp* demo_app;

@implementation eriAppDelegate

@synthesize window;
@synthesize gl_view;

- (void)update
{
	// calculate delta time
	CFTimeInterval now_time = CFAbsoluteTimeGetCurrent();
	static CFTimeInterval last_time = now_time;
	static CFTimeInterval delta_time = 0;
	delta_time = now_time - last_time;
	last_time = now_time;
		
	// App update
	demo_app->Update(delta_time);
	
	// ERI update
	ERI::Root::Ins().Update();
	
	[[gl_view openGLContext] flushBuffer];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	//[window setAcceptsMouseMovedEvents:YES];
	
	ERI::Root::Ins().Init();
	ERI::Root::Ins().renderer()->Resize([gl_view bounds].size.width, [gl_view bounds].size.height);

	demo_app = new DemoApp;
	
	// create our rendering timer
	[NSTimer scheduledTimerWithTimeInterval:(1.0f/60.0f) target:self selector:@selector(update) userInfo:nil repeats:YES];	
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication
{
	return YES;
}


@end
