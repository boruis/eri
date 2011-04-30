//
//  gl_view.m
//  eri
//
//  Created by exe on 10/17/10.
//  Copyright 2010 cobbler. All rights reserved.
//

#import "gl_view.h"

#include "root.h"
#include "renderer.h"
#include "input_mgr.h"
#include "math_helper.h"

float mouse_down_x, mouse_down_y;
float right_mouse_down_x, right_mouse_down_y;

static unsigned int GetFunctionKeyStatus(unsigned long flags)
{
	unsigned int status = 0;
	if (flags & NSShiftKeyMask) status |= ERI::FUNC_SHIFT;
	if (flags & NSControlKeyMask) status |= ERI::FUNC_CTRL;
	if (flags & NSAlternateKeyMask) status |= ERI::FUNC_ALT;
	if (flags & NSCommandKeyMask) status |= ERI::FUNC_CMD;
	
	return status;
}

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
	e.function_key_status = GetFunctionKeyStatus([event modifierFlags]);
	ERI::Root::Ins().input_mgr()->Press(e);
	
	mouse_down_x = pos.x;
	mouse_down_y = pos.y;
}

- (void)rightMouseDown:(NSEvent *)event
{
	NSPoint pos = [self convertPoint:[event locationInWindow] fromView:nil];
	
	right_mouse_down_x = pos.x;
	right_mouse_down_y = pos.y;
}

- (void)mouseMoved:(NSEvent *)event
{
	NSPoint pos = [self convertPoint:[event locationInWindow] fromView:nil];

	ERI::InputEvent e(0, pos.x, pos.y);
	e.dx = [event deltaX];
	e.dy = [event deltaY];
	ERI::Root::Ins().input_mgr()->OverMove(e);
}

- (void)mouseDragged:(NSEvent *)event
{
	NSPoint pos = [self convertPoint:[event locationInWindow] fromView:nil];
	
	ERI::InputEvent e(0, pos.x, pos.y);
	e.dx = [event deltaX];
	e.dy = [event deltaY];
	ERI::Root::Ins().input_mgr()->Move(e);
}

- (void)mouseUp:(NSEvent *)event
{
	NSPoint pos = [self convertPoint:[event locationInWindow] fromView:nil];
	
	ERI::InputEvent e(0, pos.x, pos.y);
	e.function_key_status = GetFunctionKeyStatus([event modifierFlags]);
	ERI::Root::Ins().input_mgr()->Release(e);
	
	if (ERI::Abs(pos.x - mouse_down_x) < 10 && ERI::Abs(pos.y - mouse_down_y) < 10)
	{
		ERI::Root::Ins().input_mgr()->Click(e);
	}
}

- (void)rightMouseUp:(NSEvent *)event
{
	NSPoint pos = [self convertPoint:[event locationInWindow] fromView:nil];
	
	if (ERI::Abs(pos.x - right_mouse_down_x) < 10 && ERI::Abs(pos.y - right_mouse_down_y) < 10)
	{
		ERI::InputEvent e(0, pos.x, pos.y);
		e.function_key_status = GetFunctionKeyStatus([event modifierFlags]);
		ERI::Root::Ins().input_mgr()->RightClick(e);
	}
}

- (void)scrollWheel:(NSEvent *)event
{
	NSPoint pos = [self convertPoint:[event locationInWindow] fromView:nil];
	
	ERI::InputEvent e(0, pos.x, pos.y);
	e.dx = [event deltaX];
	e.dy = [event deltaY];
	ERI::Root::Ins().input_mgr()->Scroll(e);
}

static ERI::InputKeyCode TranslateKeyCode(int event_key_code)
{
	ERI::InputKeyCode code = ERI::KEY_NONE;
	
	switch (event_key_code)
	{
		case 0x75:
			code = ERI::KEY_DELETE;
			NSLog(@"key delete!");
			break;
		case 0x33:
			code = ERI::KEY_BACKSPACE;
			NSLog(@"key backspace!");
			break;
		case 0x35:
			code = ERI::KEY_ESCAPE;
			NSLog(@"key escape!");
			break;
		case 0x7b:
			code = ERI::KEY_LEFT;
			NSLog(@"key left!");
			break;
		case 0x7c:
			code = ERI::KEY_RIGHT;
			NSLog(@"key right!");
			break;
		case 0x7d:
			code = ERI::KEY_DOWN;
			NSLog(@"key down!");
			break;
		case 0x7e:
			code = ERI::KEY_UP;
			NSLog(@"key up!");
			break;
	}
	
	return code;
}

- (void)keyDown:(NSEvent *)event
{
	// TODO: repeat handle
	
	if ([event isARepeat])
		return;
	
	NSString *characters = [event characters];
	NSLog(@"KeyDown %@\n", characters);
	
	ERI::InputKeyEvent e;
	e.characters = [characters UTF8String];
	e.code = TranslateKeyCode([event keyCode]);
	e.function_key_status = GetFunctionKeyStatus([event modifierFlags]);
	
	ERI::Root::Ins().input_mgr()->KeyDown(e);
}

- (void)keyUp:(NSEvent *)event
{
	NSString *characters = [event characters];
	NSLog(@"KeyUp %@\n", characters);
	
	ERI::InputKeyEvent e;
	e.characters = [characters UTF8String];
	e.code = TranslateKeyCode([event keyCode]);
	e.function_key_status = GetFunctionKeyStatus([event modifierFlags]);
	
	ERI::Root::Ins().input_mgr()->KeyUp(e);
}

- (void)reshape
{
	if (ERI::Root::Ins().renderer())
		ERI::Root::Ins().renderer()->Resize([self bounds].size.width, [self bounds].size.height);
}

- (void)dealloc
{
    [super dealloc];
}


@end
