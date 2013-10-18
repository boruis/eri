/*
 *  render_context.h
 *  eri
 *
 *  Created by exe on 11/28/09.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#ifndef ERI_RENDER_CONTEXT_H
#define ERI_RENDER_CONTEXT_H

namespace ERI {
	
	class RenderContext
	{
	public:
		RenderContext() {}
		virtual ~RenderContext() {}
		
		virtual bool Init() = 0;
		virtual void BackingLayer(void* layer) = 0;
		virtual void SetAsCurrent() = 0;
		virtual void Present() = 0;
	};

}

#endif // ERI_RENDER_CONTEXT_H