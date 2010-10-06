//
//  eagl_view.m
//  eri
//
//  Created by exe on 11/28/09.
//  Copyright 2009 cobbler. All rights reserved.
//

#import "eagl_view.h"

#include "root.h"
#include "renderer.h"
#include "scene_mgr.h"
#include "scene_actor.h"
#include "input_mgr.h"
#include "math_helper.h"

#import <QuartzCore/QuartzCore.h>

static bool	in_multi_move;

@implementation EAGLView

// You must implement this method
+ (Class) layerClass
{
    return [CAEAGLLayer class];
}

- (id)initWithFrame:(CGRect)frame
{
    if (self = [super initWithFrame:frame])
	{
		// Enable multipletouch support
		self.multipleTouchEnabled = YES;
		
		if ([self respondsToSelector:@selector(setContentScaleFactor:)])
		{
			[self setContentScaleFactor: ERI::Root::Ins().renderer()->content_scale()];
		}

		// Get the layer
		CAEAGLLayer *eaglLayer = (CAEAGLLayer*)self.layer;

		eaglLayer.opaque = TRUE;
		eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
										[NSNumber numberWithBool:FALSE],
										kEAGLDrawablePropertyRetainedBacking,
										kEAGLColorFormatRGBA8,
										kEAGLDrawablePropertyColorFormat,
										nil];
		
		in_multi_move = false;
    }

    return self;
}

- (void)layoutSubviews
{
	ERI::Root::Ins().renderer()->BackingLayer(self.layer);
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	CGPoint touch_pos;
	
	for (UITouch *touch in touches)
	{
		//printf("touch[%x] begin tap %d, timestamp %f\n", (unsigned int)touch, touch.tapCount, touch.timestamp);

		touch_pos = [touch locationInView:touch.view];
		[self convertPointByViewOrientation:&touch_pos];
		
		ERI::Root::Ins().input_mgr()->Press(ERI::InputEvent((unsigned int)touch, touch_pos.x, touch_pos.y));
	}
	
	//printf("now begin num %d, total num %d\n", [touches count], [[event allTouches] count]);
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	CGPoint touch_pos;
	
	for (UITouch *touch in touches)
	{
		//printf("touch[%x] end tap %d, timestamp %f\n", (unsigned int)touch, touch.tapCount, touch.timestamp);
		
		touch_pos = [touch locationInView:touch.view];
		[self convertPointByViewOrientation:&touch_pos];
		
		ERI::InputEvent event((unsigned int)touch, touch_pos.x, touch_pos.y);
		
		ERI::Root::Ins().input_mgr()->Release(event);
		
		if ([touch tapCount] == 1)
		{
			ERI::Root::Ins().input_mgr()->Click(event);
		}
		else if ([touch tapCount] == 2)
		{
			ERI::Root::Ins().input_mgr()->DoubleClick(event);
		}
	}
	
	if (([[event allTouches] count] - [touches count]) <= 1)
	{
		in_multi_move = false;
	}
	
	//printf("now end num %d, total num %d\n", [touches count], [[event allTouches] count]);
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	int now_touch_num = [[event allTouches] count];
	
	if (now_touch_num == 1)
	{
		UITouch *touch = [[touches allObjects] objectAtIndex:0];
		CGPoint touch_pos = [touch locationInView:touch.view];
		CGPoint prev_touch_pos = [touch previousLocationInView:touch.view];

		[self convertPointByViewOrientation:&touch_pos];
		[self convertPointByViewOrientation:&prev_touch_pos];
		
		ERI::Root::Ins().input_mgr()->Move(ERI::InputEvent((unsigned int)touch, touch_pos.x, touch_pos.y));
		
		//printf("touch[%x] move, timestamp %f\n", (unsigned int)touch, touch.timestamp);
	}
	else if (now_touch_num > 1)
	{
		static ERI::InputEvent events[16];

		UITouch* touch;
		CGPoint touch_pos;
		for (int i = 0; i < now_touch_num; ++i)
		{
			touch = [[[event allTouches] allObjects] objectAtIndex:i];
			touch_pos = [touch locationInView:touch.view];
			[self convertPointByViewOrientation:&touch_pos];

			events[i].uid = (unsigned int)touch;
			events[i].x = touch_pos.x;
			events[i].y = touch_pos.y;
						
			//printf("touch[%x] move, timestamp %f\n", (unsigned int)touch, touch.timestamp);
		}
		
		ERI::Root::Ins().input_mgr()->MultiMove(events, now_touch_num, !in_multi_move);
		
		in_multi_move = true;
	}
}

- (void)enableAccelerometer:(BOOL)enable withTimeInterval:(NSTimeInterval)interval
{
	UIAccelerometer *accel = [UIAccelerometer sharedAccelerometer];

	if (enable && interval > 0)
	{
		accel.delegate = self;
		accel.updateInterval = interval;
	}
	else if (accel.delegate == self)
	{
		accel.delegate = nil;
	}
}

- (void)convertPointByViewOrientation:(CGPoint*)point
{
	CGRect bounds = [self bounds];

	switch (ERI::Root::Ins().renderer()->view_orientation())
	{
		case ERI::PORTRAIT_HOME_BOTTOM:
			point->y = bounds.size.height - point->y;
			break;
		case ERI::PORTRAIT_HOME_TOP:
			point->x = bounds.size.width - point->x;
			break;
		case ERI::LANDSCAPE_HOME_RIGHT:
			{
				CGFloat orig_x = point->x;
				point->x = point->y;
				point->y = orig_x;
			}
			break;
		case ERI::LANDSCAPE_HOME_LEFT:
			{
				CGFloat orig_x = point->x;
				point->x = bounds.size.height - point->y;
				point->y = bounds.size.width - orig_x;
			}
			break;
		default:
			printf("Error! Invalid Orientation Type %d\n", ERI::Root::Ins().renderer()->view_orientation());
			break;
	}

	float content_scale = ERI::Root::Ins().renderer()->content_scale();
	point->x *= content_scale;
	point->y *= content_scale;
}

- (void)dealloc {
    [super dealloc];
}

#pragma mark Accelerometer Delegate

- (void)accelerometer:(UIAccelerometer*)accelerometer
	didAccelerate:(UIAcceleration*)acceleration
{
	ERI::Root::Ins().input_mgr()->Accelerate(ERI::Vector3(acceleration.x, acceleration.y, acceleration.z));
	
	static CFTimeInterval shake_start_time = 0;

	if (shake_start_time != 0)
	{
		if ((CFAbsoluteTimeGetCurrent() - shake_start_time) > 0.5)
		{
			shake_start_time = 0;
		}

		return;
	}
	
	if (fabsf(acceleration.x) > 1.5
        || fabsf(acceleration.y) > 1.5
        || fabsf(acceleration.z) > 1.5)
	{
		shake_start_time = CFAbsoluteTimeGetCurrent();
		ERI::Root::Ins().input_mgr()->Shake();
    }
} 

@end