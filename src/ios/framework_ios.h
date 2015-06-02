//
//  framework_ios.h
//  eri
//
//  Created by exe on 10/2/13.
//  Copyright (c) 2013 exe. All rights reserved.
//

#import "eagl_view.h"

struct FrameworkConfig
{
  FrameworkConfig() :
    custom_scale(0.f),
    use_depth_buffer(true),
    need_view_controller(true)
  {
  }
  
  float custom_scale;
  bool use_depth_buffer;
  bool need_view_controller;
};

@interface FrameworkViewController : UIViewController
@end

@interface Framework : NSObject
@property (strong, nonatomic) EAGLView* glView;
@property (strong, nonatomic) FrameworkViewController* viewController;
@property (assign, nonatomic) CFTimeInterval deltaTime;

- (instancetype)initWithFrame:(CGRect)frame config:(FrameworkConfig*)config;
- (void)run:(id)target withUpdate:(SEL)customUpdate;
- (void)stop;
- (void)logFPS:(BOOL)enable;

@end
