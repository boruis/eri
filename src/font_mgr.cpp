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

#include <sstream>

#include "root.h"
#include "texture_mgr.h"
#include "platform_helper.h"
#include "sys_helper.h"

namespace ERI {
	
int CreateUnicodeArray(const std::string& txt, bool is_utf8, uint32_t*& out_chars)
{
	int length = 0;
	
	if (is_utf8)
	{
		int max_buff_size = static_cast<int>(txt.length()) * 2; // TODO: may not enough
		out_chars = new uint32_t[max_buff_size];
		length = GetUnicodeFromUTF8(txt, max_buff_size, out_chars);
	}
	else
	{
		out_chars = new uint32_t[txt.length()];
		
		for (int i = 0; i < txt.length(); ++i)
			out_chars[i] = txt[i];
		
		length = static_cast<int>(txt.length());
	}
	
	return length;
}

void CalculateTxtSize(const std::string& txt,
					  const Font* font,
					  int font_size,
					  bool is_utf8,
					  float& width,
					  float& height)
{
	ASSERT(font);
	
	uint32_t* chars;
	int length = CreateUnicodeArray(txt, is_utf8, chars);
	
	CalculateTxtSize(chars, length, font, font_size, width, height);
	
	delete [] chars;
}

void CalculateTxtSize(const uint32_t* chars,
					  int length,
					  const Font* font,
					  int font_size,
					  float& width,
					  float& height,
					  std::vector<float>* row_widths /*= NULL*/)
{
	ASSERT(font);
	
	width = 0.0f;
	
	float size_scale = font->GetSizeScale(font_size);
	height = font->common_line_height() * size_scale;
	
	if (length == 0)
	{
		if (row_widths)
			row_widths->push_back(0);
		
		return;
	}
	
	float now_width = 0;
	for (int i = 0; i < length; ++i)
	{
		if (chars[i] == '\n')
		{
			if (now_width > width) width = now_width;
			
			if (row_widths)
				row_widths->push_back(now_width);
			
			now_width = 0;
			height += font->common_line_height() * size_scale;
		}
		else
		{
			const CharSetting& setting = font->GetCharSetting(chars[i]);
			now_width += setting.x_advance * size_scale;
		}
	}
	
	if (now_width > width) width = now_width;
	
	if (row_widths)
		row_widths->push_back(now_width);
}
	
#pragma mark Font

Font::Font()
	: texture_(NULL),
	filter_min_(FILTER_NEAREST),
	filter_mag_(FILTER_NEAREST),
	size_(0),
	common_line_height_(0),
	common_base_(0)
{
}

const CharSetting& Font::GetCharSetting(uint32_t unicode) const
{
	std::map<uint32_t, CharSetting>::const_iterator it = char_map_.find(unicode);
	
	ASSERT(it != char_map_.end());
	
	return it->second;
}
	
float Font::GetSizeScale(int want_size) const
{
	ASSERT(size_ > 0);
	
	return static_cast<float>(want_size) / size_;
}

void Font::SetTextureFilter(TextureFilter filter_min, TextureFilter filter_mag) const
{
	filter_min_ = filter_min;
	filter_mag_ = filter_mag;
}

#pragma mark FontFntScript

bool FontFntScript::Load(const std::string& path)
{
	std::string absolute_path = GetAbsolutePath(path);
	
	std::string content;
	if (!GetFileContentString(absolute_path, content))
	{
		ASSERT2(0, "Can't load FontFntScript %s", absolute_path.c_str());
		return false;
	}

	std::stringstream content_stream(content, std::ios_base::in);
	
	const int kLineLength = 256;
	char content_line[kLineLength];
	
	char tmp[32];
	content_stream.getline(content_line, kLineLength);
	sscanf(content_line, "info face=\"%[^\"]\" size=%d", tmp, &size_);
	
	content_stream.getline(content_line, kLineLength);
	sscanf(content_line, "common lineHeight=%d base=%d", &common_line_height_, &common_base_);
	
	content_stream.getline(content_line, kLineLength);
	
	char texture_name[32];
	sscanf(content_line, "page id=0 file=\"%s", texture_name);
	texture_name[strlen(texture_name) - 1] = 0;
	
	int char_num;
	content_stream.getline(content_line, kLineLength);
	sscanf(content_line, "chars count=%d", &char_num);
	
	int id;
	CharSetting setting;
	
	for (int i = 0; i <= char_num; ++i)
	{
		content_stream.getline(content_line, kLineLength);
		
		sscanf(content_line, "char id=%d x=%d y=%d width=%d height=%d xoffset=%d yoffset=%d xadvance=%d",
			   &id, &setting.x, &setting.y,
			   &setting.width, &setting.height,
			   &setting.x_offset, &setting.y_offset,
			   &setting.x_advance);
		
		char_map_.insert(std::make_pair(id, setting));
	}
	
	std::string texture_path = GetDir(path) + texture_name;
	texture_ = Root::Ins().texture_mgr()->GetTexture(texture_path);
	
	if (!texture_)
	{
		ASSERT2(0, "Can't load FontFntScript texture %s", texture_path.c_str());
		return false;
	}
	
	return true;
}

#ifdef ERI_FONT_FREETYPE
#pragma mark FontFreeType

static const double kFixDiv16 = 1.0 / (1 << 16);
static float convert_fix16(long fix)
{
	return fix * kFixDiv16;
}
static const double kFixDiv26 = 1.0 / (1 << 6);
static float convert_fix26(long fix)
{
	return (fix >> 6) + (fix & 0x0000003F) * kFixDiv26;
}
	
FontFreeType::FontFreeType(FT_Library lib_ref, int pixel_height)
	: lib_ref_(lib_ref),
	face_(NULL)
{
	ASSERT(lib_ref_);
	
	size_ = pixel_height;
	
	ASSERT(size_);
}
	
FontFreeType::~FontFreeType()
{
}

bool FontFreeType::Load(const std::string& path)
{
	std::string absolute_path = GetAbsolutePath(path);
	
	FT_Error error = FT_New_Face(lib_ref_, absolute_path.c_str(), 0, &face_);
	if (error)
	{
		ASSERT3(0, "error load face %d", error);
	}
	error = FT_Set_Pixel_Sizes(face_,  /* handle to face object */
							   0,      /* pixel_width           */
							   size_); /* pixel_height          */
	if (error)
	{
		ASSERT3(0, "error set pixel size %d", error);
	}
	
	common_line_height_ = static_cast<int>(convert_fix26(face_->size->metrics.height));
	common_base_ = static_cast<int>(convert_fix26(face_->size->metrics.ascender));

	return true;
}
	
const Texture* FontFreeType::CreateSpriteTxt(const std::string& name,
											 const std::string& txt,
											 int size,
											 bool is_pos_center,
											 bool is_utf8,
											 bool is_anti_alias,
											 int& out_width,
											 int& out_height) const
{
	uint32_t* unicodes;
    int length = CreateUnicodeArray(txt, is_utf8, unicodes);
	
	out_width = out_height = 0;
	
	FT_GlyphSlot slot = face_->glyph;
	FT_Error error;
	
	int start_x, start_y;
	start_x = start_y = 0;
	
	std::vector<CharSetting> settings(length);
	
	for (int i = 0; i < length; ++i)
	{
		if (unicodes[i] == '\n')
		{
			start_x = 0;
			start_y += common_line_height_;
			continue;
		}
		
		error = FT_Load_Char(face_, unicodes[i], FT_LOAD_DEFAULT);
		if (error)
		{
			ASSERT3(0, "error load char %d", error);
		}
		
		CharSetting& setting = settings[i];
		
		setting.x = start_x;
		setting.y = start_y;
		setting.width = static_cast<int>(convert_fix26(slot->metrics.width));
		setting.height = static_cast<int>(convert_fix26(slot->metrics.height));
		setting.x_offset = static_cast<int>(convert_fix26(slot->metrics.horiBearingX));
		setting.y_offset = common_base_ - static_cast<int>(convert_fix26(slot->metrics.horiBearingY));
		setting.x_advance = static_cast<int>(convert_fix26(slot->metrics.horiAdvance));

		start_x += setting.x_advance;
		
		if (start_x > out_width) out_width = start_x;
		if ((start_y + common_line_height_) > out_height)
			out_height = start_y + common_line_height_;
	}

	int tex_width = next_power_of_2(out_width);
	int tex_height = next_power_of_2(out_height);
	int pixel_num = tex_width * tex_height;
	
	char* buff = static_cast<char*>(malloc(pixel_num * 4));
	memset(buff, 0xFF, pixel_num * 4);
	
	char* p;
	
	for (int i = 0; i < pixel_num; ++i)
	{
		p = &buff[i * 4];
		p[3] = 0;
	}
	
	bool is_byte_bitmap;
	for (int i = 0; i < length; ++i)
	{
		if (unicodes[i] == '\n')
			continue;
		
		error = FT_Load_Char(face_, unicodes[i],
							 is_anti_alias ? FT_LOAD_RENDER : FT_LOAD_RENDER | FT_LOAD_MONOCHROME);
		if (error)
		{
			ASSERT3(0, "error load char %d", error);
		}
		
		is_byte_bitmap = (slot->bitmap.width == slot->bitmap.pitch);

		CharSetting& setting = settings[i];
		
		for (int row = 0; row < slot->bitmap.rows; ++row)
		{
			p = &buff[((setting.y + setting.y_offset + row) * tex_width + setting.x + setting.x_offset) * 4];
			for (int col = 0; col < slot->bitmap.width; ++col)
			{
				if (is_byte_bitmap)
				{
					p[col * 4 + 3] = slot->bitmap.buffer[row * slot->bitmap.pitch + col];
				}
				else
				{
					char c = slot->bitmap.buffer[row * slot->bitmap.pitch + static_cast<int>(col / 8)];
					p[col * 4 + 3] = ((c >> (7 - col % 8)) & 0x01) ? 255 : 0;
				}
			}
		}
	}
	
	delete [] unicodes;

	return Root::Ins().texture_mgr()->CreateTexture(name, tex_width, tex_height, buff);
}

#endif // ERI_FONT_FREETYPE

#if defined(ERI_TEXTURE_READER_UIKIT) || defined(ERI_TEXTURE_READER_ANDROID)
#pragma mark FontSys

class FontSys : public Font
{
public:
	virtual bool Load(const std::string& path);

	virtual const Texture* CreateSpriteTxt(const std::string& name,
		const std::string& txt,
		int size,
		bool is_pos_center,
		bool is_utf8,
		bool is_anti_alias,
		int& out_width,
		int& out_height) const;

	virtual float GetSizeScale(int want_size) const { return 1.f; }

private:
	std::string name_;
};

bool FontSys::Load(const std::string& path)
{
	name_ = path;
	return true;
}

const Texture* FontSys::CreateSpriteTxt(const std::string& name,
		const std::string& txt,
		int size,
		bool is_pos_center,
		bool is_utf8,
		bool is_anti_alias,
		int& out_width,
		int& out_height) const
{
	Vector2 acture_size;
	std::string tex_name(name);
	Root::Ins().texture_mgr()->ReleaseTexture(tex_name);
	const Texture* tex = Root::Ins().texture_mgr()->GenerateTxtTexture(txt, name_, size, is_pos_center, acture_size, &tex_name);
	
	ASSERT(tex);
	
	out_width = acture_size.x;
	out_height = acture_size.y;
	
	return tex;
}

#endif // defined(ERI_TEXTURE_READER_UIKIT) || defined(ERI_TEXTURE_READER_ANDROID)

#pragma FontMgr
	
FontMgr::FontMgr()
#ifdef ERI_FONT_FREETYPE
	: ft_lib_(NULL)
#endif
{
}

FontMgr::~FontMgr()
{
	for (std::map<std::string, Font*>::iterator it = font_map_.begin(); it != font_map_.end(); ++it)
	{
		delete it->second;
	}

#ifdef ERI_FONT_FREETYPE
	if (ft_lib_)
	{
		FT_Error error = FT_Done_FreeType(ft_lib_);
		if (error)
		{
			ASSERT3(0, "error done freetype %d", error);
		}
	}
#endif
}

const Font* FontMgr::GetFont(const std::string& path, int want_pixel_height /*= 32*/)
{
	std::string font_name = GetFileName(path);
	
	std::map<std::string, Font*>::iterator it = font_map_.find(font_name);
	if (it == font_map_.end())
	{
		Font* font = NULL;
		
		std::string extension = GetFileNameExtension(font_name);
		if (extension.compare("fnt") == 0)
		{
			font = new FontFntScript;
		}
#ifdef ERI_FONT_FREETYPE
		else if (extension.compare("ttf") == 0)
		{
			if (!ft_lib_)
			{
				FT_Error error = FT_Init_FreeType(&ft_lib_);
				if (error)
				{
					ASSERT3(0, "error init freetype %d", error);
					return NULL;
				}
			}
			
			font = new FontFreeType(ft_lib_, want_pixel_height);
		}
#endif
		else
		{
#if defined(ERI_TEXTURE_READER_UIKIT) || defined(ERI_TEXTURE_READER_ANDROID)
			font = new FontSys;
#else
			ASSERT2(0, "Invalid font path %s", path.c_str());
			return NULL;
#endif
		}
		
		if (!font)
		{
			ASSERT2(0, "Can't create font %s", path.c_str());
		}
		
		if (!font->Load(path))
		{
			delete font;
			return NULL;
		}
		
		font_map_.insert(std::make_pair(font_name, font));
		
		return font;
	}
	else
	{
		return it->second;
	}
}

}
