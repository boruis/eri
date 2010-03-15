/*
 *  font_mgr.h
 *  eri
 *
 *  Created by exe on 12/21/09.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#ifndef ERI_FONT_MGR_H
#define ERI_FONT_MGR_H

#include "texture_mgr.h"

#include <string>
#include <map>

namespace ERI {
	
	struct CharSetting
	{
		int	x, y;
		int width, height;
		int x_offset, y_offset;
		int x_advance;
	};
	
	struct Font
	{
		const Texture*	texture;
		int				size;
		int				common_line_height;
		int				common_base;
		
		mutable	TextureFilter	filter_min;
		mutable	TextureFilter	filter_mag;
		
		std::map<int, CharSetting>	char_map;
		
		const CharSetting& GetCharSetting(int id) const;
	};
	
	class FontMgr
		{
		public:
			~FontMgr();
			
			const Font* GetFont(const std::string& name);
			
		private:
			std::map<std::string, Font*>	font_map_;
		};
	
}

#endif // ERI_FONT_MGR_H
