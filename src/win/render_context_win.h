/*
 *  render_context_win.h
 *  eri
 *
 *  Created by exe on 08/29/10.
 *  Copyright 2010 cobbler. All rights reserved.
 *
 */

#ifndef ERI_RENDER_DATA_WIN_H
#define ERI_RENDER_DATA_WIN_H

#include "render_context.h"

namespace ERI {

	struct WinInfo;

	class RenderContextWin : public RenderContext
	{
	public:
		RenderContextWin();
		virtual ~RenderContextWin();

		virtual bool Init(int version);
		virtual void BackingLayer(void* layer);
		virtual void SetAsCurrent();
		virtual void Present();

	private:
		WinInfo*	win_info_;
	};
}

#endif // ERI_RENDER_DATA_WIN_H