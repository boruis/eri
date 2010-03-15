/*
 *  render_context_iphone.h
 *  eri
 *
 *  Created by exe on 11/28/09.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#ifndef ERI_RENDER_CONTEXT_IPHONE_H
#define ERI_RENDER_CONTEXT_IPHONE_H

#include "render_context.h"

#ifdef __OBJC__
@class EAGLContext;
#endif

namespace ERI {

	class RenderContextIphone : public RenderContext
	{
	public:
		RenderContextIphone();
		virtual ~RenderContextIphone();
		
		virtual bool Init(int version);
		virtual void BackingLayer(void* layer);
		virtual void SetAsCurrent();
		virtual void Present();
		
	private:
#ifdef __OBJC__
		EAGLContext* context_;
#endif
	};
}

#endif // ERI_RENDER_CONTEXT_IPHONE_H