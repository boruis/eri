/*
 *  render_context_iphone.cpp
 *  eri
 *
 *  Created by exe on 11/28/09.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#include "render_context_ios.h"

#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/EAGLDrawable.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

namespace ERI {
	
	RenderContextIphone::RenderContextIphone() : context_(nil)
	{
	}
	
	RenderContextIphone::~RenderContextIphone()
	{
		if (context_)
		{
			if ([EAGLContext currentContext] == context_)
				[EAGLContext setCurrentContext:nil];
			
			[context_ release];
		}
	}
	
	bool RenderContextIphone::Init(int version)
	{
		switch (version)
		{
			case 1:
				context_ = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
				break;
			case 2:
				context_ = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
				break;
		}
		
		if (!context_ || ![EAGLContext setCurrentContext:context_])
		{
			return false;
		}

		return true;
	}
	
	void RenderContextIphone::BackingLayer(void* layer)
	{
		[context_ renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(CAEAGLLayer*)layer];
	}
	
	void RenderContextIphone::SetAsCurrent()
	{
		[EAGLContext setCurrentContext:context_];
	}
	
	void RenderContextIphone::Present()
	{
		[context_ presentRenderbuffer:GL_RENDERBUFFER_OES];
	}

}