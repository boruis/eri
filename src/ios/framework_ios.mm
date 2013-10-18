//
//  framework_ios.cpp
//  raid
//
//  Created by exe on 10/2/13.
//  Copyright (c) 2013 exe. All rights reserved.
//

#include "framework_ios.h"

#import "eagl_view.h"

#include "root.h"
#include "renderer.h"

@implementation Framework

- (id)init
{
  if (self = [super init])
  {
    ERI::Root::Ins().Init();

    self.gl_view = [[EAGLView alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    self.gl_view.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    
    is_running_ = NO;
    keep_delta_time_ = YES;
  }
  
  return self;
}

- (void)update
{
  // calculate delta time
	static CFTimeInterval now_time = 0.0;
  now_time = updator_.timestamp;
	
	static CFTimeInterval prev_time = now_time;
	
	if (keep_delta_time_)
	{
		delta_time_ = now_time - prev_time;
	}
	else
	{
		delta_time_ = 0.0;
		keep_delta_time_ = YES;
	}
	
	prev_time = now_time;
  
  [target_ performSelector:custom_update_];
  
	if (is_running_)
  {
		ERI::Root::Ins().Update();
    
    frame_count_ += 1;
    
    frame_pass_time_ += delta_time_;
    if (frame_pass_time_ >= 1.0)
    {
      NSLog(@"fps %.2f", frame_count_ / frame_pass_time_);
      frame_pass_time_ = 0.0;
      frame_count_ = 0;
    }
  }
}

- (void)Run:(id)target withUpdate:(SEL)custom_update
{
  if (!is_running_)
  {
    target_ = target;
    custom_update_ = custom_update;
    
    updator_ = [CADisplayLink displayLinkWithTarget:self selector:@selector(update)];
    [updator_ addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    
		is_running_ = YES;
		keep_delta_time_ = NO;
    
    frame_pass_time_ = 0.0;
    frame_count_ = 0;
	}
}

- (void)Stop
{
  if (is_running_)
  {
    [updator_ invalidate];
    updator_ = nil;

    is_running_ = NO;
  }
}

- (CFTimeInterval)DeltaTime
{
  return delta_time_;
}

@end
