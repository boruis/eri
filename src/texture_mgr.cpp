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
#include "root.h"

namespace ERI {
	
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
	
}
