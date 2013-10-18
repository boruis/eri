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

@interface Framework : NSObject
{
  CADisplayLink* updator_;

  id target_;
  SEL custom_update_;

  BOOL is_running_, keep_delta_time_;
  CFTimeInterval delta_time_;
  
  CFTimeInterval frame_pass_time_;
  int frame_count_;
}

@property (strong, nonatomic) EAGLView* gl_view;

- (void)Run:(id)target withUpdate:(SEL)custom_update;
- (void)Stop;
- (CFTimeInterval)DeltaTime;
@end

#endif /* defined(__raid__framework_ios__) */
