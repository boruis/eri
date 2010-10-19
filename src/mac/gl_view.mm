//
//  gl_view.m
//  eri
//
//  Created by exe on 10/17/10.
//  Copyright 2010 cobbler. All rights reserved.
//

#import "gl_view.h"

#include "root.h"
#include "input_mgr.h"
#include "math_helper.h"

float mouse_down_x, mouse_down_y;

@implementation GLView

- (id)initWithFrame:(NSRect)frame
{
    if ((self = [super initWithFrame:frame]))
	{
        // Initialization code
    }
    return self;
}

- (BOOL)acceptsFirstResponder
{
	return YES;
}

- (void)mouseDown:(NSEvent *)event
{
	NSPoint pos = [self convertPoint:[event locationInWindow] fromView:nil];

	ERI::InputEvent e(0, pos.x, pos.y);
	ERI::Root::Ins().input_mgr()->Press(e);
	
	mouse_down_x = pos.x;
	mouse_down_y = pos.y;
}

- (void)mouseMoved:(NSEvent *)event
{
	NSPoint pos = [self convertPoint:[event locationInWindow] fromView:nil];

	ERI::InputEvent e(0, pos.x, pos.y);
	ERI::Root::Ins().input_mgr()->Move(e);
}

- (void)mouseDragged:(NSEvent *)event
{
	NSPoint pos = [self convertPoint:[event locationInWindow] fromView:nil];
	
	ERI::InputEvent e(0, pos.x, pos.y);
	ERI::Root::Ins().input_mgr()->Move(e);
}

- (void)mouseUp:(NSEvent *)event
{
	NSPoint pos = [self convertPoint:[event locationInWindow] fromView:nil];
		
	ERI::InputEvent e(0, pos.x, pos.y);
	ERI::Root::Ins().input_mgr()->Release(e);
	
	if (ERI::Abs(pos.x - mouse_down_x) < 10 && ERI::Abs(pos.y - mouse_down_y) < 10)
	{
		ERI::Root::Ins().input_mgr()->Click(e);
	}
}

- (void)scrollWheel:(NSEvent *)event
{
	NSLog(@"scroll delta %f %f %f\n", [event deltaX], [event deltaY], [event deltaZ]);
}

- (void)keyDown:(NSEvent *)event
{
    NSString *characters = [event characters];
	NSLog(@"key %@\n", characters);
}

- (void)dealloc {
    [super dealloc];
}


@end
