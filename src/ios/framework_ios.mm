//
//  framework_ios.cpp
//  eri
//
//  Created by exe on 10/2/13.
//  Copyright (c) 2013 exe. All rights reserved.
//

#include "framework_ios.h"

#import "eagl_view.h"

#include "root.h"
#include "renderer.h"

@implementation FrameworkViewController
{
  NSMutableArray* supported_orientations_;
}

// <= iOS 5 handle
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
  if (!supported_orientations_)
  {
    supported_orientations_ = [NSMutableArray array];
    
    NSArray* orientations = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"UISupportedInterfaceOrientations"];
    for (NSString* orientation in orientations)
    {
      if ([orientation isEqualToString:@"UIInterfaceOrientationPortrait"])
        [supported_orientations_ addObject:@(UIInterfaceOrientationPortrait)];
      else if ([orientation isEqualToString:@"UIInterfaceOrientationPortraitUpsideDown"])
        [supported_orientations_ addObject:@(UIInterfaceOrientationPortraitUpsideDown)];
      else if ([orientation isEqualToString:@"UIInterfaceOrientationLandscapeLeft"])
        [supported_orientations_ addObject:@(UIInterfaceOrientationLandscapeLeft)];
      else if ([orientation isEqualToString:@"UIInterfaceOrientationLandscapeRight"])
        [supported_orientations_ addObject:@(UIInterfaceOrientationLandscapeRight)];
    }
  }
  
  for (NSNumber* orientation in supported_orientations_)
  {
    if (interfaceOrientation == [orientation integerValue])
      return YES;
  }
  
  return NO;
}
@end

@implementation Framework
{
  CADisplayLink* updator_;
  
  id target_;
  SEL custom_update_;
  
  BOOL is_running_, keep_delta_time_;
  
  BOOL log_fps_;
  CFTimeInterval frame_pass_time_;
  int frame_count_;
}

- (instancetype)initWithFrame:(CGRect)frame config:(FrameworkConfig*)config
{
  self = [super init];
  if (self)
  {
    FrameworkConfig* default_config = NULL;
    if (!config)
    {
      default_config = new FrameworkConfig;
      config = default_config;
    }
    
    ERI::Root::Ins().Init(config->use_depth_buffer);
    
    if ([UIScreen instancesRespondToSelector:@selector(scale)])
    {
      float content_scale = [[UIScreen mainScreen] scale];
      if (config->custom_scale > 0.f)
        content_scale = config->custom_scale;
    
      ERI::Root::Ins().renderer()->set_content_scale(content_scale);
    }
    
    _glView = [[EAGLView alloc] initWithFrame:frame];
    _glView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    
    if (config->need_view_controller)
    {
      _viewController = [[FrameworkViewController alloc] init];
      [_viewController.view addSubview:_glView];
    }
    
    if (default_config) delete default_config;
    
    //
    
    is_running_ = NO;
    keep_delta_time_ = YES;
    log_fps_ = NO;
  }
  
  return self;
}

- (void)dealloc
{
  ERI::Root::DestroyIns();
  
#if !__has_feature(objc_arc)
  [_viewController release];
  [_glView release];
  
  [super dealloc];
#endif
}

- (void)update
{
  // calculate delta time
	static CFTimeInterval now_time = 0.0;
  now_time = updator_.timestamp;
	
	static CFTimeInterval prev_time = now_time;
	
	if (keep_delta_time_)
	{
		self.deltaTime = now_time - prev_time;
	}
	else
	{
		self.deltaTime = 0.0;
		keep_delta_time_ = YES;
	}
	
	prev_time = now_time;
  
  [target_ performSelector:custom_update_];
  
	if (is_running_)
  {
		ERI::Root::Ins().Update();
    
    if (log_fps_)
    {
      frame_count_ += 1;
      
      frame_pass_time_ += self.deltaTime;
      if (frame_pass_time_ >= 1.0)
      {
        NSLog(@"fps %.2f", frame_count_ / frame_pass_time_);
        frame_pass_time_ = 0.0;
        frame_count_ = 0;
      }
    }
  }
}

- (void)run:(id)target withUpdate:(SEL)customUpdate
{
  if (!is_running_)
  {
    target_ = target;
    custom_update_ = customUpdate;
    
    updator_ = [CADisplayLink displayLinkWithTarget:self selector:@selector(update)];
    [updator_ addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    
		is_running_ = YES;
		keep_delta_time_ = NO;
    
    frame_pass_time_ = 0.0;
    frame_count_ = 0;
	}
}

- (void)stop
{
  if (is_running_)
  {
    [updator_ invalidate];
    updator_ = nil;

    is_running_ = NO;
  }
}

- (void)logFPS:(BOOL)enable
{
  log_fps_ = enable;
  frame_pass_time_ = 0.0;
  frame_count_ = 0;
}

@end
