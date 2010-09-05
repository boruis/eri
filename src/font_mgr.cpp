/*
 *  font_mgr.cpp
 *  eri
 *
 *  Created by exe on 12/21/09.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#include "pch.h"

#include "font_mgr.h"
#include "root.h"
#include "texture_mgr.h"
#include "platform_helper.h"

#include <sstream>

#if ERI_PLATFORM == ERI_PLATFORM_ANDROID
#include <jni.h>
#include <android/log.h>

extern JNIEnv*	global_env;
extern jobject	global_renderer;
extern jclass	global_renderer_cls;
#endif

namespace ERI {
	
	const CharSetting& Font::GetCharSetting(int id) const
	{
		std::map<int, CharSetting>::const_iterator it = char_map.find(id);
		ASSERT(it != char_map.end());
		return it->second;
	}

	FontMgr::~FontMgr()
	{
		for (std::map<std::string, Font*>::iterator it = font_map_.begin(); it != font_map_.end(); ++it)
		{
			delete it->second;
		}
		font_map_.clear();
	}
	
	const Font* FontMgr::GetFont(const std::string& name)
	{
		std::map<std::string, Font*>::iterator it = font_map_.find(name);
		if (it == font_map_.end())
		{
			// TODO: default path setting is bad

#if ERI_PLATFORM == ERI_PLATFORM_IOS
			std::string font_path(GetResourcePath());
			font_path += "/media/";
#else
			std::string font_path("media/");
#endif
			font_path += (name + ".fnt");
			
			std::string content;

#if ERI_PLATFORM == ERI_PLATFORM_ANDROID
			jmethodID mid = global_env->GetMethodID(global_renderer_cls, "GetTxtFileContent", "(Ljava/lang/String;)Ljava/lang/String;");
			jstring s = static_cast<jstring>(global_env->CallObjectMethod(global_renderer, mid, global_env->NewStringUTF(font_path.c_str())));
			
			const char* cstr = global_env->GetStringUTFChars(s, 0);
			
			content = cstr;
	
			global_env->ReleaseStringUTFChars(s, cstr);
#else
			FILE* file = fopen(font_path.c_str(), "r");
			ASSERT(file);
			
			fseek(file, 0, SEEK_END);
			long file_size = ftell(file);
			rewind(file);
			char* buffer = (char*)malloc(sizeof(char) * file_size);
			ASSERT(buffer);
			size_t result = fread(buffer, sizeof(char), file_size, file);
			ASSERT(result == file_size);

			content = buffer;
			
			fclose(file);
			free(buffer);
#endif
			
			std::stringstream content_stream(content, std::ios_base::in);
			
			Font* font = new Font;

			char txt_line[256];
			
			content_stream.getline(txt_line, 256);

			char tmp[32];
			sscanf(txt_line, "info face=\"%s size=%d", tmp, &font->size);

			content_stream.getline(txt_line, 256);
			
			sscanf(txt_line, "common lineHeight=%d base=%d", &font->common_line_height, &font->common_base);
			
			content_stream.getline(txt_line, 256);
			
			char texture_name[32];
			sscanf(txt_line, "page id=0 file=\"%s", texture_name);
			texture_name[strlen(texture_name) - 1] = 0;
			
			content_stream.getline(txt_line, 256);

			int char_num;
			sscanf(txt_line, "chars count=%d", &char_num);
			
			int id;
			CharSetting setting;
			
			for (int i = 0; i <= char_num; ++i)
			{
				content_stream.getline(txt_line, 256);
				
				sscanf(txt_line, "char id=%d x=%d y=%d width=%d height=%d xoffset=%d yoffset=%d xadvance=%d",
					   &id, &setting.x, &setting.y, &setting.width, &setting.height, &setting.x_offset, &setting.y_offset, &setting.x_advance);
				
				font->char_map.insert(std::make_pair(id, setting));
			}
			
			font->texture = Root::Ins().texture_mgr()->GetTexture(std::string("media/") + texture_name);
			if (!font->texture)
			{
				delete font;
				return NULL;
			}
			
			font->filter_min = FILTER_NEAREST;
			font->filter_mag = FILTER_NEAREST;
			
			font_map_.insert(std::make_pair(name, font));
			
			return font;
		}
		else
		{
			return it->second;
		}
	}

}
