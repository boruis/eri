/*
 *  platform_helper.h
 *  eri
 *
 *  Created by exe on 11/28/09.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#ifndef ERI_PLATFORM_HELPER_H
#define ERI_PLATFORM_HELPER_H

#include "renderer.h"

#include <string>
#include <vector>
#include <map>

namespace ERI {

	const char* GetResourcePath();
	const char* GetHomePath();
	std::string GetWritePath();
	
	const char* GetStringFileContent(const std::string& file_path);
	
	void SetDeviceOrientation(ViewOrientation orientation);
	
	struct TextureAtlasUnit
	{
		int x, y, width, height;
	};
	
	typedef std::vector<TextureAtlasUnit> TextureAtlasArray;
	typedef std::map<std::string, TextureAtlasUnit> TextureAtlasMap;

	bool GetTextureAtlasArray(const std::string& name, TextureAtlasArray& out_array);
	bool GetTextureAtlasMap(const std::string& name, TextureAtlasMap& out_map);
	
	int GetUnicodeFromUTF8(const std::string& str, int max_buff_length, uint32_t* buff);
	
	const char* GetLocale();
}

#endif // ERI_PLATFORM_HELPER_H
