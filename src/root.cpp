/*
 *  root.cpp
 *  eri
 *
 *  Created by exe on 11/28/09.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#include "pch.h"

#include "root.h"

#ifdef ERI_RENDERER_ES1
#include "renderer_es1.h"
#endif
#ifdef ERI_RENDERER_ES2
#include "renderer_es2.h"
#include "shader_mgr.h"
#endif

#include "scene_mgr.h"
#include "input_mgr.h"
#include "texture_mgr.h"
#include "font_mgr.h"

namespace ERI {
	
	Root* Root::ins_ptr_ = NULL;
	
	Root::Root() :
		renderer_(NULL),
		scene_mgr_(NULL),
		input_mgr_(NULL),
		texture_mgr_(NULL),
		font_mgr_(NULL),
		shader_mgr_(NULL),
		window_handle_(NULL)
	{
	}
	
	Root::~Root()
	{
#ifdef ERI_RENDERER_ES2
		if (shader_mgr_) delete shader_mgr_;
#endif

		if (font_mgr_) delete font_mgr_;
		if (texture_mgr_) delete texture_mgr_;
		if (input_mgr_) delete input_mgr_;
		if (scene_mgr_) delete scene_mgr_;
		if (renderer_) delete renderer_;
	}
	
	void Root::Init(bool use_depth_buffer /*= true*/)
	{
#ifdef ERI_RENDERER_ES2
		renderer_ = new RendererES2;
		if (renderer_->Init(use_depth_buffer))
		{
			shader_mgr_ = new ShaderMgr;
		}
		else
		{
			delete renderer_;
			renderer_ = NULL;
		}
#endif

#ifdef ERI_RENDERER_ES1
		if (!renderer_)
		{
			renderer_ = new RendererES1;
			
			if (!renderer_->Init(use_depth_buffer))
			{
				delete renderer_;
				renderer_ = NULL;
			}
		}
#endif

		ASSERT(renderer_);
		
		scene_mgr_ = new SceneMgr;
		input_mgr_ = new InputMgr;
		texture_mgr_ = new TextureMgr;
		font_mgr_ = new FontMgr;
	}
	
	void Root::Update()
	{
		renderer_->RenderStart();
		scene_mgr_->Render(renderer_);
		renderer_->RenderEnd();
	}

}
