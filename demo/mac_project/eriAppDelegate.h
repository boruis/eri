//
//  eriAppDelegate.h
//  eri
//
//  Created by exe on 10/16/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class GLView;

@interface eriAppDelegate : NSObject <NSApplicationDelegate> {
    NSWindow *window;
	NSOpenGLView *gl_view;
}

@property (assign) IBOutlet NSWindow *window;
@property (assign) IBOutlet NSOpenGLView *gl_view;

@end
