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

@implementation GLView


- (id)initWithFrame:(CGRect)frame {
    if ((self = [super initWithFrame:frame])) {
        // Initialization code
    }
    return self;
}

/*
// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect {
    // Drawing code
}
*/

- (BOOL)acceptsFirstResponder
{
	return YES;
}

- (void)mouseDown:(NSEvent *)event
{
	NSPoint pos = [self convertPoint:[event locationInWindow] fromView:nil];

	ERI::InputEvent e(0, pos.x, pos.y);
	ERI::Root::Ins().input_mgr()->Press(e);
	
	NSLog(@"mouse down\n");
}

- (void)mouseMoved:(NSEvent *)event
{
	NSPoint pos = [self convertPoint:[event locationInWindow] fromView:nil];

	ERI::InputEvent e(0, pos.x, pos.y);
	ERI::Root::Ins().input_mgr()->Move(e);
	
	NSLog(@"mouse moved\n");
}

- (void)mouseDragged:(NSEvent *)event
{
	NSPoint pos = [self convertPoint:[event locationInWindow] fromView:nil];
	
	ERI::InputEvent e(0, pos.x, pos.y);
	ERI::Root::Ins().input_mgr()->Move(e);

	NSLog(@"mouse dragged\n");
}

- (void)mouseUp:(NSEvent *)event
{
	NSPoint pos = [self convertPoint:[event locationInWindow] fromView:nil];
		
	ERI::InputEvent e(0, pos.x, pos.y);
	ERI::Root::Ins().input_mgr()->Release(e);

	// TODO: more precise judgement
	ERI::Root::Ins().input_mgr()->Click(e);
	
	NSLog(@"mouse up\n");
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
