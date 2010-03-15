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

float	zoom_distance;
bool	in_zoom;

@implementation EAGLView

// You must implement this method
+ (Class) layerClass
{
    return [CAEAGLLayer class];
}

- (id)initWithFrame:(CGRect)frame {
    if (self = [super initWithFrame:frame]) {
		
		// Enable multipletouch support
		self.multipleTouchEnabled = YES;

		// Get the layer
		CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;

		eaglLayer.opaque = TRUE;
		eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
										[NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
		
		in_zoom = false;
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
		//printf("touch begin tap %d, timestamp %f\n", touch.tapCount, touch.timestamp);

		touch_pos = [touch locationInView:touch.view];
		[self convertPointByViewOrientation:&touch_pos];
		
		ERI::Root::Ins().input_mgr()->Press(touch_pos.x, touch_pos.y);
	}
	
	//printf("now begin num %d, total num %d\n", [touches count], [[event allTouches] count]);
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	CGPoint touch_pos;
	
	for (UITouch *touch in touches)
	{
		//printf("touch end tap %d, timestamp %f\n", touch.tapCount, touch.timestamp);
		
		touch_pos = [touch locationInView:touch.view];
		[self convertPointByViewOrientation:&touch_pos];
		
		ERI::Root::Ins().input_mgr()->Release(touch_pos.x, touch_pos.y);
		ERI::Root::Ins().input_mgr()->Click(touch_pos.x, touch_pos.y);
	}
	
	in_zoom = false;
	
	//printf("now end num %d, total num %d\n", [touches count], [[event allTouches] count]);
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	int now_touch_num = [[event allTouches] count];
	//printf("touch move num %d\n", now_touch_num);
	
	if (now_touch_num == 1)
	{
		in_zoom = false;
		
		UITouch *t = [[touches allObjects] objectAtIndex:0];
		CGPoint touch_pos = [t locationInView:t.view];
		CGPoint prev_touch_pos = [t previousLocationInView:t.view];

		[self convertPointByViewOrientation:&touch_pos];
		[self convertPointByViewOrientation:&prev_touch_pos];
		
		ERI::Root::Ins().input_mgr()->Move(touch_pos.x, touch_pos.y);
		
//		ERI::Vector2 delta_pos(touch_pos.x - prev_touch_pos.x, touch_pos.y - prev_touch_pos.y);
//		ERI::CameraActor* cam = ERI::Root::Ins().scene_mgr()->current_cam(); 
//		ERI::Vector2 cam_pos = cam->GetPos();
//		cam_pos -= (delta_pos * (1 / cam->zoom()));
//		cam->SetPos(cam_pos.x, cam_pos.y);
	}
	else if (now_touch_num > 1)
	{
		UITouch* t = [[[event allTouches] allObjects] objectAtIndex:0];
		CGPoint touch_pos_1 = [t locationInView:t.view];
		t = [[[event allTouches] allObjects] objectAtIndex:1];
		CGPoint touch_pos_2 = [t locationInView:t.view];
		
		float delta_x = touch_pos_1.x - touch_pos_2.x;
		float delta_y = touch_pos_1.y - touch_pos_2.y;
		float distance = sqrt(delta_x * delta_x + delta_y * delta_y);

		if (!in_zoom)
		{
			in_zoom = true;
		}
		else
		{
//			float cam_zoom = ERI::Root::Ins().scene_mgr()->current_cam()->zoom();
//			cam_zoom += (distance - zoom_distance) * 0.005f;
//			if (cam_zoom < 0.1f) cam_zoom = 0.1f;
//			ERI::Root::Ins().scene_mgr()->current_cam()->SetZoom(cam_zoom);
		}

		zoom_distance = distance;
	}
	else
	{
		in_zoom = false;
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
}

- (void)dealloc {
    [super dealloc];
}


@end
