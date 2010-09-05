//
//  eriAppDelegate.h
//  eri
//
//  Created by exe on 11/28/09.
//  Copyright cobbler 2009. All rights reserved.
//

#import <UIKit/UIKit.h>

@class EAGLView;

@interface eriAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
	EAGLView *gl_view;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;

@end

