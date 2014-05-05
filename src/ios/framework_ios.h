//
//  framework_ios.h
//  raid
//
//  Created by exe on 10/2/13.
//  Copyright (c) 2013 exe. All rights reserved.
//

#ifndef __raid__framework_ios__
#define __raid__framework_ios__

#import "eagl_view.h"

@interface FrameworkViewController : UIViewController
@end

@interface Framework : NSObject
@property (strong, nonatomic) EAGLView* gl_view;
@property (strong, nonatomic) FrameworkViewController* view_controller;
- (id)initWithFrame:(CGRect)frame needViewController:(BOOL)need_view_controller;
- (void)LogFPS:(BOOL)enable;
- (void)Run:(id)target withUpdate:(SEL)custom_update;
- (void)Stop;
- (CFTimeInterval)DeltaTime;
@end

#endif /* defined(__raid__framework_ios__) */
