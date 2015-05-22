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

#include <string>

#include "renderer.h"
#include "texture_atlas_mgr.h"

struct CallbackInfoColor;

namespace ERI {

	const char* GetResourcePath();
	const char* GetHomePath();
	const char* GetBundlePath();
	std::string GetWritePath();
	
	void SetDeviceOrientation(ViewOrientation orientation);
	
	bool GetTextureAtlasArray(const std::string& name, TextureAtlasArray& out_array);
	bool GetTextureAtlasMap(const std::string& name, TextureAtlasMap& out_map);
	
	int GetUnicodeFromUTF8(const std::string& str, int max_buff_length, uint32_t* buff);
	
	const char* GetLocale();
	
	enum InfoType
	{
		INFO_NORMAL = 0,
		INFO_WARNING,
		INFO_CRITICAL
	};
	
	void ShowMessageBox(const char* msg, const char* additional_msg = NULL, InfoType info_type = INFO_NORMAL);
	bool ShowConfirmMessageBox(const char* msg, const char* additional_msg = NULL, const char* action_txt = NULL, InfoType info_type = INFO_NORMAL);
	
	bool GetOpenFilePath(std::string& out_path, const char* filter = NULL, const char* title_msg = NULL);
	bool GetSaveFilePath(std::string& out_path, const char* filter = NULL, const char* title_msg = NULL);
	
	void ShowColorPicker(const Color& color, const CallbackInfoColor& callback);

}

#endif // ERI_PLATFORM_HELPER_H
