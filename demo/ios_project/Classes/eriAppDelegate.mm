//
//  eriAppDelegate.m
//  eri
//
//  Created by exe on 11/28/09.
//  Copyright cobbler 2009. All rights reserved.
//

#import "eriAppDelegate.h"

#import "eagl_view.h"
#include "root.h"
#include "renderer.h"

#include "demo_app.h"

DemoApp* demo_app;

@implementation eriAppDelegate

@synthesize window;

- (void) update
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
}

- (void)applicationDidFinishLaunching:(UIApplication *)application
{
	// init ERI
	ERI::Root::Ins().Init();
	ERI::Root::Ins().renderer()->SetViewOrientation(ERI::LANDSCAPE_HOME_RIGHT);
	
	if ([UIScreen instancesRespondToSelector:@selector(scale)])
	{
		ERI::Root::Ins().renderer()->set_content_scale([[UIScreen mainScreen] scale]);
	}	

	// create the OpenGL view and add it to the window
	gl_view = [[EAGLView alloc] initWithFrame:[[UIScreen mainScreen] bounds]];	
	[window addSubview:gl_view];
	
	// init App
	demo_app = new DemoApp;
	
	// create our rendering timer
	[NSTimer scheduledTimerWithTimeInterval:0 target:self selector:@selector(update) userInfo:nil repeats:YES];	
	
    // Override point for customization after application launch
    [window makeKeyAndVisible];
}

- (void)dealloc {
	
	delete demo_app;
	
	[gl_view release];
    [window release];
    [super dealloc];
}


@end
