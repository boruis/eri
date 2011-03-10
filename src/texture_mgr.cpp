/*
 *  texture_mgr.cpp
 *  eri
 *
 *  Created by exe on 11/30/09.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#include "pch.h"

#include "texture_mgr.h"

#if ERI_PLATFORM == ERI_PLATFORM_IOS
#include "ios/texture_reader_uikit.h"
#include "texture_reader_libpng.h"
#elif ERI_PLATFORM == ERI_PLATFORM_ANDROID
#include "texture_reader_bitmap_factory.h"
#else
#include "texture_reader_freeimage.h"
#endif

#include "texture_reader.h"
#include "renderer.h"
#include "scene_mgr.h"
#include "scene_actor.h"
#include "root.h"

namespace ERI {
	
#pragma mark Texture
	
	Texture::Texture(int _id, int _width, int _height) :
		id(_id),
		width(_width),
		height(_height),
		data(NULL),
		bind_frame_buffer(0)
	{
	}
	
	Texture::~Texture()
	{
		if (data) free(data);
	}
	
	void Texture::CopyPixels(const void* _data)
	{
		if (!data) data = calloc(width * height * 4, sizeof(unsigned char));
		
		memcpy(data, _data, width * height * 4 * sizeof(unsigned char));
	}
	
	bool Texture::GetPixelColor(Color& out_color, int x, int y) const
	{
		if (!data) return false;
		
		if (x < 0) x = 0;
		if (x >= width) x = width - 1;
		if (y < 0) y = 0;
		if (y >= height) y = height - 1;
		
		unsigned char* pixel = static_cast<unsigned char*>(data);
		pixel += (width * y + x) * 4;
		out_color.r = pixel[0] / 255.0f;
		out_color.g = pixel[1] / 255.0f;
		out_color.b = pixel[2] / 255.0f;
		out_color.a = pixel[3] / 255.0f;
		
		return true;
	}
	
	void Texture::ReleaseFromRenderer()
	{
		if (bind_frame_buffer)
			Root::Ins().renderer()->ReleaseRenderToTexture(id, bind_frame_buffer);
		else
			Root::Ins().renderer()->ReleaseTexture(id);
		
		id = 0;
	}
	
#pragma mark TextureMgr
	
	TextureMgr::~TextureMgr()
	{
		for (int i = 0; i < preload_textures_.size(); ++i)
		{
			delete preload_textures_[i].reader;
		}
		
		for (std::map<std::string, Texture*>::iterator it = texture_map_.begin(); it != texture_map_.end(); ++it)
		{
			// TODO: release?
			
			delete it->second;
		}
	}
	
	void TextureMgr::PreloadTexture(const std::string& resource_path)
	{
		std::map<std::string, Texture*>::iterator it = texture_map_.find(resource_path);
		if (it == texture_map_.end())
		{
#if ERI_PLATFORM == ERI_PLATFORM_IOS
			PreloadTextureInfo info;
			info.path = resource_path;
			info.reader = new TextureReaderLibPNG(resource_path, false);
			preload_textures_.push_back(info);
#endif
		}
	}
	
	void TextureMgr::ConstructPreloadTextures()
	{
		int num = preload_textures_.size();
		for (int i = 0; i < num; ++i)
		{
			PreloadTextureInfo& info = preload_textures_[i];
			
			info.reader->Generate();
			
			// TODO: check texture invalid number, maybe use int -1 is better
			
			if (info.reader->texture_id() != 0)
			{
				Texture* tex = new Texture(info.reader->texture_id(), info.reader->width(), info.reader->height());
				texture_map_.insert(std::make_pair(info.path, tex));
			}
			
			delete info.reader;
		}
		
		preload_textures_.clear();
	}
	
	const Texture* TextureMgr::GetTexture(const std::string& resource_path, bool keep_texture_data /*= false*/)
	{
		std::map<std::string, Texture*>::iterator it = texture_map_.find(resource_path);
		if (it == texture_map_.end())
		{
#if ERI_PLATFORM == ERI_PLATFORM_IOS
			TextureReaderUIImage reader(resource_path, true);
			//TextureReaderLibPNG reader(resource_path, true);
#elif ERI_PLATFORM == ERI_PLATFORM_ANDROID
			TextureReaderBitmapFactory reader(resource_path);
#else
			TextureReaderFreeImage reader(resource_path, true);
#endif

			// TODO: check texture invalid number, maybe use int -1 is better
			
			if (reader.texture_id() == 0)
				return NULL;
			
			Texture* tex = new Texture(reader.texture_id(), reader.width(), reader.height());
			
			if (keep_texture_data)
			{
				tex->CopyPixels(reader.texture_data());
			}
			
			texture_map_.insert(std::make_pair(resource_path, tex));
	
			return tex;
		}
		else
		{
			return it->second;
		}
	}
	
	const Texture* TextureMgr::CreateTexture(const std::string& name, int width, int height, const void* data)
	{
		ASSERT(!name.empty() && width > 0 && height > 0 && data);
		
		std::map<std::string, Texture*>::iterator it = texture_map_.find(name);
		
		ASSERT(it == texture_map_.end());
		
		unsigned int texture_id = Root::Ins().renderer()->GenerateTexture(data, width, height, RGBA);
		if (texture_id == 0)
			return NULL;
		
		Texture* tex = new Texture(texture_id, width, height);
		
		texture_map_.insert(std::make_pair(name, tex));
		
		return tex;
	}
	
	void TextureMgr::UpdateTexture(Texture* tex, const void* data)
	{
		ASSERT(tex && tex->id > 0 && data);
		
		Root::Ins().renderer()->UpdateTexture(tex->id, data, tex->width, tex->height, RGBA);
	}
	
	const Texture* TextureMgr::GetTxtTexture(const std::string& txt, const std::string& font_name, float font_size, float w, float h)
	{
		std::map<std::string, Texture*>::iterator it = texture_map_.find(txt + "_txt");
		if (it == texture_map_.end())
		{
#if ERI_PLATFORM == ERI_PLATFORM_IOS
			TextureReaderUIFont reader(txt, font_name, font_size, w, h);
#else
			TextureReader reader(true);
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
	
	const Texture* TextureMgr::GenerateRenderToTexture(int width, int height)
	{
		int bind_frame_buffer = 0;
		int texture_id = Root::Ins().renderer()->GenerateRenderToTexture(width, height, bind_frame_buffer);
		
		// TODO: check texture invalid number, maybe use int -1 is better
		
		if (texture_id != 0)
		{
			Texture* texture = new Texture(texture_id, width, height);

			texture->bind_frame_buffer = bind_frame_buffer;
			
			static int serial_number = 0;
			char key[16];
			sprintf(key, "%d_render2tex", serial_number++);
						
			texture_map_.insert(std::make_pair(key, texture));
			
			return texture;
		}
		
		return NULL;
	}

	void TextureMgr::ReleaseTexture(const std::string& name)
	{
		std::map<std::string, Texture*>::iterator it = texture_map_.find(name);
		if (it != texture_map_.end())
		{
			Texture* texture = it->second;
			
			texture->ReleaseFromRenderer();
			texture_map_.erase(it);
			delete texture;
		}
	}
	
	void TextureMgr::ReleaseTexture(const Texture* texture)
	{
		ASSERT(texture);
		
		std::map<std::string, Texture*>::iterator it;
		for (it = texture_map_.begin(); it != texture_map_.end(); ++it)
		{
			if ((*it).second == texture)
			{
				it->second->ReleaseFromRenderer();
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
		render_cam_(render_cam),
		out_copy_pixels_(NULL)
	{
	}
	
	RenderToTexture::~RenderToTexture()
	{
		Release();
	}
	
	void RenderToTexture::Init()
	{
		ASSERT(!texture_);

		texture_ = Root::Ins().texture_mgr()->GenerateRenderToTexture(width_, height_);
	}
	
	void RenderToTexture::Release()
	{
		if (texture_)
		{
			Root::Ins().texture_mgr()->ReleaseTexture(texture_);
		}
	}
	
	void RenderToTexture::ProcessRender()
	{
		ASSERT(texture_);

		Renderer* renderer = Root::Ins().renderer();
		SceneMgr* scene_mgr = Root::Ins().scene_mgr();
		
		renderer->EnableRenderToBuffer(width_, height_, texture_->bind_frame_buffer);
		
		CameraActor* current_cam = scene_mgr->current_cam();
		
		if (render_cam_ && render_cam_ != current_cam)
		{
			scene_mgr->SetCurrentCam(render_cam_);
		}
		else
		{
			scene_mgr->OnViewportResize();
		}
		
		//

		renderer->RenderStart();
		scene_mgr->Render(renderer);
		renderer->CopyTexture(texture_->id);
		
		//
		
		if (out_copy_pixels_)
		{
			renderer->CopyPixels(out_copy_pixels_, 0, 0, width_, height_);
		}

		//
		
		renderer->RestoreRenderToBuffer();
		
		if (render_cam_ && render_cam_ != current_cam)
		{
			scene_mgr->SetCurrentCam(current_cam);
		}
		else
		{
			scene_mgr->OnViewportResize();
		}
	}
	
	void RenderToTexture::CopyPixels(void* out_copy_pixels)
	{
		out_copy_pixels_ = out_copy_pixels;
	}
	
}
