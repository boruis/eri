/*
 *  texture_mgr.h
 *  eri
 *
 *  Created by exe on 11/30/09.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#ifndef ERI_TEXTURE_MGR_H
#define ERI_TEXTURE_MGR_H

#include <string>
#include <map>

namespace ERI {
	
	enum TextureFilter
	{
		FILTER_NEAREST,
		FILTER_LINEAR
	};
	
	struct Texture
	{
		Texture(int _id, int _width, int _height) :
			id(_id),
			width(_width),
			height(_height),
			filter_min(FILTER_NEAREST),
			filter_mag(FILTER_NEAREST)
		{
		}
		
		int				id;
		int				width;
		int				height;
		
		mutable TextureFilter	filter_min;
		mutable TextureFilter	filter_mag;
	};
	
	class TextureMgr
	{
	public:
		~TextureMgr();
		
		const Texture* GetTexture(const std::string& resource_path);
		const Texture* GetTxtTexture(const std::string& txt, const std::string& font_name, float font_size, float w, float h);
		
	private:
		std::map<std::string, Texture*>	texture_map_;
	};
	
}

#endif // ERI_TEXTURE_MGR_H
