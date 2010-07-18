/*
 *  texture_mgr.cpp
 *  eri
 *
 *  Created by exe on 11/30/09.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#include "texture_mgr.h"

#ifdef OS_ANDROID
#include "texture_reader_bitmap_factory.h"
#else
#include "texture_reader_uikit.h"
#endif

#include "texture_reader.h"
#include "renderer.h"
#include "scene_mgr.h"
#include "scene_actor.h"
#include "root.h"

namespace ERI {
	
#pragma mark TextureMgr
	
	TextureMgr::~TextureMgr()
	{
		for (std::map<std::string, Texture*>::iterator it = texture_map_.begin(); it != texture_map_.end(); ++it)
		{
			delete it->second;
		}
		texture_map_.clear();
	}
	
	const Texture* TextureMgr::GetTexture(const std::string& resource_path)
	{
		std::map<std::string, Texture*>::iterator it = texture_map_.find(resource_path);
		if (it == texture_map_.end())
		{
#ifdef OS_ANDROID
			TextureReaderBitmapFactory reader(resource_path);
#else
			TextureReaderUIImage reader(resource_path);
#endif

			// TODO: check texture invalid number, maybe use int -1 is better
			
			if (reader.texture_id() == 0)
				return NULL;
			
			Texture* tex = new Texture(reader.texture_id(), reader.width(), reader.height());

			texture_map_.insert(std::make_pair(resource_path, tex));
			
			return tex;
		}
		else
		{
			return it->second;
		}
	}
	
	const Texture* TextureMgr::GetTxtTexture(const std::string& txt, const std::string& font_name, float font_size, float w, float h)
	{
		std::map<std::string, Texture*>::iterator it = texture_map_.find(txt + "_txt");
		if (it == texture_map_.end())
		{
#ifdef OS_ANDROID
			TextureReader reader;
#else
			TextureReaderUIFont reader(txt, font_name, font_size, w, h);
#endif
			
			// TODO: check texture invalid number, maybe use int -1 is better
			
			if (reader.texture_id() == 0)
				return NULL;
			
			Texture* tex = new Texture(reader.texture_id(), reader.width(), reader.height());
			
			texture_map_.insert(std::make_pair(txt + "_txt", tex));

			return tex;
		}
		else
		{
			return it->second;
		}
	}
	
	const Texture* TextureMgr::GenerateRenderToTexture(int width, int height, int& out_frame_buffer)
	{
		int texture_id = Root::Ins().renderer()->GenerateRenderToTexture(width, height, out_frame_buffer);
		
		// TODO: check texture invalid number, maybe use int -1 is better
		
		if (texture_id != 0)
		{
			Texture* tex = new Texture(texture_id, width, height);
			
			static int serial_number = 0;
			char key[6];
			sprintf(key, "%d_render2tex", serial_number++);
						
			texture_map_.insert(std::make_pair(key, tex));
			
			return tex;
		}
		
		return NULL;
	}
	
	void TextureMgr::ReleaseTexture(const Texture* texture)
	{
		ASSERT(texture);
		
		// TODO: map key?
		
		std::map<std::string, Texture*>::iterator it;
		for (it = texture_map_.begin(); it != texture_map_.end(); ++it)
		{
			if ((*it).second == texture)
			{
				Root::Ins().renderer()->ReleaseTexture(texture->id);
				texture_map_.erase(it);
				delete texture;
				return;
			}
		}
	}
	
#pragma mark RenderToTexture
	
	RenderToTexture::RenderToTexture(int width, int height, CameraActor* render_cam /*= NULL*/) :
		width_(width),
		height_(height),
		texture_(NULL),
		frame_buffer_(0),
		render_cam_(render_cam)
	{
	}
	
	RenderToTexture::~RenderToTexture()
	{
		Release();
	}
	
	void RenderToTexture::Init()
	{
		ASSERT(!texture_);

		texture_ = Root::Ins().texture_mgr()->GenerateRenderToTexture(width_, height_, frame_buffer_);
	}
	
	void RenderToTexture::Release()
	{
		if (texture_)
		{
			Root::Ins().texture_mgr()->ReleaseTexture(texture_);
		}
			
		if (frame_buffer_)
		{
			Root::Ins().renderer()->ReleaseFrameBuffer(frame_buffer_);
		}
	}
	
	void RenderToTexture::ProcessRender()
	{
		ASSERT(texture_);
		
		Root::Ins().renderer()->EnableRenderToBuffer(width_, height_, frame_buffer_);
		
		CameraActor* current_cam = Root::Ins().scene_mgr()->current_cam();
		
		if (render_cam_ && render_cam_ != current_cam)
		{
			Root::Ins().scene_mgr()->SetCurrentCam(render_cam_);
		}
		else
		{
			Root::Ins().scene_mgr()->OnRenderResize();
		}
		
		Root::Ins().scene_mgr()->Render(Root::Ins().renderer());
		
		Root::Ins().renderer()->RestoreRenderToBuffer();
		
		if (render_cam_ && render_cam_ != current_cam)
		{
			Root::Ins().scene_mgr()->SetCurrentCam(current_cam);
		}
		else
		{
			Root::Ins().scene_mgr()->OnRenderResize();
		}
	}
	
}
