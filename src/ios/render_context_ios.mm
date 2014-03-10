/*
 *  render_context_iphone.cpp
 *  eri
 *
 *  Created by exe on 11/28/09.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#include "pch.h"

#include "render_context_ios.h"

#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/EAGLDrawable.h>

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
			
#if !__has_feature(objc_arc)
			[context_ release];
#endif
		}
	}
	
	bool RenderContextIphone::Init()
	{
#ifdef ERI_RENDERER_ES1
		context_ = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
#else
		context_ = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
#endif
		
		if (!context_ || ![EAGLContext setCurrentContext:context_])
		{
			return false;
		}

		return true;
	}
	
	void RenderContextIphone::BackingLayer(const void* layer)
	{
#ifdef ERI_RENDERER_ES1
		[context_ renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(__bridge CAEAGLLayer*)layer];
#else
		[context_ renderbufferStorage:GL_RENDERBUFFER fromDrawable:(__bridge CAEAGLLayer*)layer];
#endif
	}
	
	void RenderContextIphone::SetAsCurrent()
	{
		[EAGLContext setCurrentContext:context_];
	}
	
	void RenderContextIphone::Present()
	{
#ifdef ERI_RENDERER_ES1
		[context_ presentRenderbuffer:GL_RENDERBUFFER_OES];
#else
		[context_ presentRenderbuffer:GL_RENDERBUFFER];
#endif
	}

}