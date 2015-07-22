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

#ifdef ERI_TEXTURE_READER_LIBPNG
#include "texture_reader_libpng.h"
#endif

#ifdef ERI_TEXTURE_READER_FREEIMAGE
#include "texture_reader_freeimage.h"
#endif

#ifdef ERI_TEXTURE_READER_UIKIT
#include "ios/texture_reader_uikit.h"
#endif

#ifdef ERI_TEXTURE_READER_PVR
#include "ios/texture_reader_pvr.h"
#endif

#ifdef ERI_TEXTURE_READER_ANDROID
#include "android/texture_reader_android.h"
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
		alpha_premultiplied(false)
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
			PreloadTextureInfo info;
			info.path = resource_path;

#ifdef ERI_TEXTURE_READER_PVR
			if (resource_path.substr(resource_path.length() - 4, 4).compare(".pvr") == 0)
				info.reader = new TextureReaderPVR(resource_path, false);
			else
#endif
#if defined(ERI_TEXTURE_READER_UIKIT)
#	if defined(ERI_TEXTURE_READER_LIBPNG)
				if (resource_path.substr(resource_path.length() - 4, 4).compare(".jpg") == 0)
					info.reader = new TextureReaderUIImage(resource_path, false);
				else
					info.reader = new TextureReaderLibPNG(resource_path, false);
#	else
				info.reader = new TextureReaderUIImage(resource_path, false);
#	endif
#elif defined(ERI_TEXTURE_READER_LIBPNG)
				info.reader = new TextureReaderLibPNG(resource_path, false);
#elif defined(ERI_TEXTURE_READER_FREEIMAGE)
				info.reader = new TextureReaderFreeImage(resource_path, false);
#else
      ASSERT(0);
#endif

			preload_textures_.push_back(info);
		}
	}
	
	void TextureMgr::ConstructPreloadTextures()
	{
		size_t num = preload_textures_.size();
		for (int i = 0; i < num; ++i)
		{
			PreloadTextureInfo& info = preload_textures_[i];
			
			info.reader->Generate();
			
			// TODO: check texture invalid number, maybe use int -1 is better
			
			if (info.reader->texture_id() != 0)
			{
				Texture* tex = new Texture(info.reader->texture_id(), info.reader->width(), info.reader->height());
				tex->alpha_premultiplied = info.reader->alpha_premultiplied();
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
			TextureReader* reader;
      
#ifdef ERI_TEXTURE_READER_PVR
			if (resource_path.substr(resource_path.length() - 4, 4).compare(".pvr") == 0)
				reader = new TextureReaderPVR(resource_path, true);
			else
#endif
#if defined(ERI_TEXTURE_READER_UIKIT)
#	if defined(ERI_TEXTURE_READER_LIBPNG)
				if (resource_path.substr(resource_path.length() - 4, 4).compare(".png") == 0)
					reader = new TextureReaderLibPNG(resource_path, true);
				else
#	endif
				reader = new TextureReaderUIImage(resource_path, true);
#elif defined(ERI_TEXTURE_READER_LIBPNG)
				reader = new TextureReaderLibPNG(resource_path, true);
#elif defined(ERI_TEXTURE_READER_FREEIMAGE)
				reader = new TextureReaderFreeImage(resource_path, true);
#else
      ASSERT(0);
#endif

			// TODO: check texture invalid number, maybe use int -1 is better
			
			if (reader->texture_id() == 0)
			{
				delete reader;
				return NULL;
			}
			
			Texture* tex = new Texture(reader->texture_id(), reader->width(), reader->height());
			tex->alpha_premultiplied = reader->alpha_premultiplied();
			
			if (keep_texture_data)
			{
				tex->CopyPixels(reader->texture_data());
			}
			
			texture_map_.insert(std::make_pair(resource_path, tex));
			
			delete reader;
	
			return tex;
		}
		else
		{
			return it->second;
		}
	}
	
	const Texture* TextureMgr::CreateTexture(const std::string& name, int width, int height, const void* data, PixelFormat pixel_format /*= RGBA*/)
	{
		ASSERT(!name.empty() && width > 0 && height > 0);
		
		std::map<std::string, Texture*>::iterator it = texture_map_.find(name);

		if (it != texture_map_.end())
		{
			it->second->width = width;
			it->second->height = height;

			UpdateTexture(it->second, data);

			return it->second;
		}

		unsigned int texture_id = Root::Ins().renderer()->GenerateTexture(data, width, height, pixel_format);
		if (texture_id == 0)
			return NULL;
		
		Texture* tex = new Texture(texture_id, width, height);
		
		texture_map_.insert(std::make_pair(name, tex));
		
		return tex;
	}
  
	const Texture* TextureMgr::CreateTexture(const std::string& name, TextureReader* reader)
	{
		ASSERT(!name.empty() && reader);
		
		// TODO: reader may not keep texture data, can't always update texture here
		
		ASSERT(texture_map_.find(name) == texture_map_.end());
		
		if (reader->texture_id() <= 0)
			return NULL;
		
		Texture* tex = new Texture(reader->texture_id(), reader->width(), reader->height());
		tex->alpha_premultiplied = reader->alpha_premultiplied();
		
		texture_map_.insert(std::make_pair(name, tex));
		
		return tex;
	}
	
	void TextureMgr::UpdateTexture(Texture* tex, const void* data)
	{
		ASSERT(tex && tex->id > 0 && data);
		
		Root::Ins().renderer()->UpdateTexture(tex->id, data, tex->width, tex->height, RGBA);
	}

	bool TextureMgr::ReleaseTexture(const std::string& name)
	{
		std::map<std::string, Texture*>::iterator it = texture_map_.find(name);
		if (it != texture_map_.end())
		{
			Texture* texture = it->second;
			
			texture->ReleaseFromRenderer();
			texture_map_.erase(it);
			delete texture;
      
			return true;
		}

		return false;
	}
	
	bool TextureMgr::ReleaseTexture(const Texture* texture)
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
				
				return true;
			}
		}

		return false;
	}
	
#pragma mark RenderToTexture
	
	RenderToTexture::RenderToTexture(int x, int y, int width, int height, CameraActor* render_cam /*= NULL*/) :
		x_(x),
		y_(y),
		width_(width),
		height_(height),
		texture_(NULL),
		bind_frame_buffer_(0),
		is_own_texture_(false),
		pixel_format_(RGBA),
		render_cam_(render_cam),
		default_cam_(NULL),
		out_copy_pixels_(NULL)
	{
	}
	
	RenderToTexture::~RenderToTexture()
	{
		Release();
	}
	
	void RenderToTexture::Init(const Texture* exist_texture /*= NULL*/)
	{
		if (exist_texture)
		{
			if (texture_ && is_own_texture_)
				Root::Ins().texture_mgr()->ReleaseTexture(texture_);
			
			texture_ = exist_texture;
			is_own_texture_ = false;
		}
		else
		{
			if (NULL == texture_ || !is_own_texture_)
			{
				char name[32];
				sprintf(name, "render2tex_%p", this);
				texture_ = Root::Ins().texture_mgr()->CreateTexture(name, width_, height_, NULL, pixel_format_);
				is_own_texture_ = true;
			}
		}
		
		if (bind_frame_buffer_ == 0)
			bind_frame_buffer_ = Root::Ins().renderer()->GenerateFrameBuffer();
		
		Root::Ins().renderer()->BindTextureToFrameBuffer(texture_->id, bind_frame_buffer_);
		Root::Ins().renderer()->BindDefaultFrameBuffer();
	}
	
	void RenderToTexture::Release()
	{
		if (bind_frame_buffer_ > 0)
		{
			Root::Ins().renderer()->ReleaseFrameBuffer(bind_frame_buffer_);
			bind_frame_buffer_ = 0;
		}
		
		if (texture_)
		{
			if (is_own_texture_)
				Root::Ins().texture_mgr()->ReleaseTexture(texture_);
			
			texture_ = NULL;
		}
	}
	
	void RenderToTexture::PreProcess()
	{
		Root::Ins().renderer()->EnableRenderToBuffer(x_, y_, width_, height_, bind_frame_buffer_);
		
		// TODO: how to handle layer's cam?
		
		default_cam_ = Root::Ins().scene_mgr()->default_cam();
		
		if (render_cam_ && render_cam_ != default_cam_)
		{
			Root::Ins().scene_mgr()->set_default_cam(render_cam_);
		}
		else
		{
			Root::Ins().scene_mgr()->OnViewportResize(false);
		}
		
		Root::Ins().renderer()->RenderStart();
	}
	
	void RenderToTexture::PostProcess()
	{
#if defined(ERI_RENDERER_ES1) && ERI_PLATFORM != ERI_PLATFORM_IOS
		ASSERT(texture_);
		Root::Ins().renderer()->CopyTexture(texture_->id, pixel_format_);
#endif
		
		if (out_copy_pixels_)
		{
			Root::Ins().renderer()->CopyPixels(out_copy_pixels_, 0, 0, width_, height_, pixel_format_);
		}
	
		//
		
		Root::Ins().renderer()->RestoreRenderToBuffer();
		
		if (render_cam_ && render_cam_ != default_cam_)
		{
			Root::Ins().scene_mgr()->set_default_cam(default_cam_);
		}
		else
		{
			Root::Ins().scene_mgr()->OnViewportResize(false);
		}
	}
	
	void RenderToTexture::ProcessRender()
	{
		PreProcess();
		Root::Ins().scene_mgr()->Render(Root::Ins().renderer());
		PostProcess();
	}
	
	void RenderToTexture::CopyPixels(void* out_copy_pixels)
	{
		out_copy_pixels_ = out_copy_pixels;
	}
	
}
