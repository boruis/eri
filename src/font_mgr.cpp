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

#ifdef ERI_TEXTURE_READER_UIKIT
#include "ios/texture_reader_uikit.h"
#endif

#ifdef ERI_TEXTURE_READER_ANDROID
#include "android/texture_reader_android.h"
#endif

#include "texture_reader.h"

namespace ERI {
	
int CreateUnicodeArray(const TxtData& data, uint32_t*& out_chars)
{
	if (data.str.empty())
		return 0;

	int length = 0;
	
	if (data.is_utf8)
	{
		int max_buff_size = static_cast<int>(data.str.length()) * 2; // TODO: may not enough
		out_chars = new uint32_t[max_buff_size];
		length = GetUnicodeFromUTF8(data.str, max_buff_size, out_chars);
	}
	else
	{
		out_chars = new uint32_t[data.str.length()];
		
		for (int i = 0; i < data.str.length(); ++i)
			out_chars[i] = data.str[i];
		
		length = static_cast<int>(data.str.length());
	}
	
	return length;
}
	
// TODO: FontSys handle

void CalculateTxtSize(const TxtData& data,
					  const Font* font,
					  int font_size,
					  float max_width,
					  float& out_width,
					  float& out_height)
{
	ASSERT(font);
	
	if (data.str.empty())
	{
		out_width = out_height = 0.f;
		return;
	}
	
	uint32_t* chars;
	int length = CreateUnicodeArray(data, chars);
	
	CalculateTxtSize(chars, length, font, font_size, max_width, out_width, out_height);
	
	delete [] chars;
}

void CalculateTxtSize(const uint32_t* chars,
					  int length,
					  const Font* font,
					  int font_size,
					  float max_width,
					  float& out_width,
					  float& out_height,
					  std::vector<float>* out_row_widths /*= NULL*/)
{
	ASSERT(font);
	
	out_width = out_height = 0.f;

	if (length == 0)
	{
		if (out_row_widths)
			out_row_widths->push_back(0.f);
		
		return;
	}
	
	float size_scale = font->GetSizeScale(font_size);
	out_height = font->common_line_height() * size_scale;
		
	float now_width = 0;
	for (int i = 0; i < length; ++i)
	{
		if (chars[i] == '\n')
		{
			if (now_width > out_width) out_width = now_width;
			
			if (out_row_widths)
				out_row_widths->push_back(now_width);
			
			now_width = 0;
			out_height += font->common_line_height() * size_scale;
		}
		else
		{
			const CharSetting& setting = font->GetCharSetting(chars[i]);
      
			if (max_width > 0.f &&
				(now_width + setting.x_advance * size_scale) > max_width)
			{
				if (now_width > out_width) out_width = now_width;

				if (out_row_widths)
					out_row_widths->push_back(now_width);

				now_width = 0;
				out_height += font->common_line_height() * size_scale;
			}
      
			now_width += setting.x_advance * size_scale;
		}
	}
	
	if (now_width > out_width) out_width = now_width;
	
	if (out_row_widths)
		out_row_widths->push_back(now_width);
}
	
#pragma mark Font

Font::Font()
	: texture_(NULL),
	filter_min_(FILTER_NEAREST),
	filter_mag_(FILTER_NEAREST),
	wrap_s_(WRAP_REPEAT),
	wrap_t_(WRAP_REPEAT),
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

void Font::SetTextureWrap(ERI::TextureWrap wrap_s, ERI::TextureWrap wrap_t) const
{
	wrap_s_ = wrap_s;
	wrap_t_ = wrap_t;
}

#pragma mark FontFntScript
  
class FontFntScript : public Font
{
public:
	virtual bool Load(const std::string& path);
	
	virtual bool is_atlas() const { return true; }
};

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

class FontFreeType : public Font
{
public:
	FontFreeType(FT_Library lib_ref, int pixel_height);
	virtual ~FontFreeType();
	
	virtual bool Load(const std::string& path);
	
	virtual const CharSetting& GetCharSetting(uint32_t unicode) const;
	
	virtual const Texture* CreateSpriteTxt(const std::string& tex_name,
		const TxtData& data,
		int font_size,
		int max_width,
		int& out_width,
		int& out_height) const;
	
	virtual float GetSizeScale(int want_size) const;
	
	virtual bool is_utf8() const { return true; }
	
private:
	void ChangeFaceSize(int want_size) const;
  
	FT_Library	lib_ref_;
	FT_Face		face_;
};
  
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
	
	ASSERT(size_ > 0);
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
		ASSERT2(0, "error load face %d", error);
	}
	
	ChangeFaceSize(size_);

	return true;
}
  
const CharSetting& FontFreeType::GetCharSetting(uint32_t unicode) const
{
	static CharSetting setting;
	FT_Error error = FT_Load_Char(face_, unicode, FT_LOAD_DEFAULT);
	if (error)
	{
		ASSERT2(0, "error load char %d", error);
	}
	
	FT_GlyphSlot slot = face_->glyph;
	
	setting.x_advance = static_cast<int>(convert_fix26(slot->metrics.horiAdvance));
	setting.width = static_cast<int>(convert_fix26(slot->metrics.width));
	setting.height = static_cast<int>(convert_fix26(slot->metrics.height));
	setting.x_offset = static_cast<int>(convert_fix26(slot->metrics.horiBearingX));
	setting.y_offset = common_base_ - static_cast<int>(convert_fix26(slot->metrics.horiBearingY));

	return setting;
}
	
const Texture* FontFreeType::CreateSpriteTxt(const std::string& tex_name,
											 const TxtData& data,
											 int font_size,
											 int max_width,
											 int& out_width,
											 int& out_height) const
{
	out_width = out_height = 0;

	if (data.str.empty())
		return NULL;

	uint32_t* unicodes;
	int length = CreateUnicodeArray(data, unicodes);
  
	if (font_size != size_)
		ChangeFaceSize(font_size);
	
	FT_GlyphSlot slot = face_->glyph;
	FT_Error error;
		
	std::vector<CharSetting> settings;
  
	if (max_width > 0 && data.line_break == LB_TRUNCATE_HEAD)
	{
		std::vector<CharSetting> invert_settings;
		std::vector<uint32_t> invert_unicodes;
		
		CharSetting setting;
		int current_width = 0;
		
		for (int i = length - 1; i >= 0; --i)
		{
			if (unicodes[i] == '\n')
			{
				setting.x_advance = 0;
			}
			else
			{
				setting = GetCharSetting(unicodes[i]);
				
				if ((current_width + setting.x_advance) > max_width)
					break;
			}

			invert_settings.push_back(setting);
			invert_unicodes.push_back(unicodes[i]);
			current_width += setting.x_advance;
		}

		if (invert_settings.size() < length)
		{
			CharSetting period_setting = GetCharSetting('.');
			
			while ((current_width + period_setting.x_advance * 3) > max_width)
			{
				current_width -= invert_settings.back().x_advance;
				invert_settings.pop_back();
				invert_unicodes.pop_back();
			}

			for (int i = 0; i < 3; ++i)
			{
				invert_settings.push_back(period_setting);
				invert_unicodes.push_back('.');
				current_width += period_setting.x_advance;
			}
			
			int prev_length = length;
			length = static_cast<int>(invert_settings.size());
			
			if (length > prev_length)
			{
				delete [] unicodes;
				unicodes = new uint32_t[length];
			}
		}
		
		int start_x = 0;
		settings.resize(length);
		for (int i = 0; i < length; ++i)
		{
			settings[i] = invert_settings[length - 1 - i];
			settings[i].x = start_x;
			start_x += settings[i].x_advance;
			
			unicodes[i] = invert_unicodes[length - 1 - i];
		}
		
		out_width = current_width;
		out_height = common_line_height_;
	}
	else if (max_width > 0 && data.line_break == LB_TRUNCATE_TAIL)
	{
		CharSetting setting;
		int current_width = 0;
		
		for (int i = 0; i < length; ++i)
		{
			if (unicodes[i] == '\n')
			{
				setting.x_advance = 0;
			}
			else
			{
				setting = GetCharSetting(unicodes[i]);
				
				if ((current_width + setting.x_advance) > max_width)
					break;
			}

			setting.x = current_width;
			current_width += setting.x_advance;
			settings.push_back(setting);
		}
		
		if (settings.size() < length)
		{
			CharSetting period_setting = GetCharSetting('.');
			
			while ((current_width + period_setting.x_advance * 3) > max_width)
			{
				current_width -= settings.back().x_advance;
				settings.pop_back();
			}
			
			if ((settings.size() + 3) > length)
			{
				delete [] unicodes;
				unicodes = new uint32_t[settings.size() + 3];
			}
			
			for (int i = 0; i < 3; ++i)
			{
				period_setting.x = current_width;
				current_width += period_setting.x_advance;
				settings.push_back(period_setting);
				unicodes[settings.size() - 1] = '.';
			}
			
			length = static_cast<int>(settings.size());
		}
		
		out_width = current_width;
		out_height = common_line_height_;
	}
	else
	{
		int start_x, start_y;
		start_x = start_y = 0;

		std::vector<int> line_width, line_end;

		settings.resize(length);

		for (int i = 0; i < length; ++i)
		{
			if (unicodes[i] == '\n')
			{
				if (data.is_pos_center)
				{
					line_width.push_back(start_x);
					line_end.push_back(i);
				}
				
				start_x = 0;
				start_y += common_line_height_;
				continue;
			}
			
			CharSetting& setting = settings[i];

			setting = GetCharSetting(unicodes[i]);

			if (max_width > 0 && (start_x + setting.x_advance) > max_width)
			{
				if (data.is_pos_center)
				{
					line_width.push_back(start_x);
					line_end.push_back(i - 1);
				}
				
				start_x = 0;
				start_y += common_line_height_;
			}
			
			setting.x = start_x;
			setting.y = start_y;

			start_x += setting.x_advance;
			
			if (start_x > out_width) out_width = start_x;
			if ((start_y + common_line_height_) > out_height)
				out_height = start_y + common_line_height_;
		}
		
		if (data.is_pos_center && length > 0)
		{
			line_width.push_back(start_x);
			line_end.push_back(length - 1);
			
			int start_c  = 0;
			for (int i = 0; i < line_width.size(); ++i)
			{
				int offset_x = (out_width - line_width[i]) / 2;
				
				for (int c = start_c; c <= line_end[i]; ++c)
					settings[c].x += offset_x;
				
				start_c = line_end[i] + 1;
			}
		}
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
							 data.is_anti_alias ? FT_LOAD_RENDER : FT_LOAD_RENDER | FT_LOAD_MONOCHROME);
		if (error)
		{
			ASSERT2(0, "error load char %d", error);
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
	
	const Texture* tex = Root::Ins().texture_mgr()->CreateTexture(tex_name, tex_width, tex_height, buff);
	
	free(buff);

	return tex;
}

float FontFreeType::GetSizeScale(int want_size) const
{
	if (want_size != size_)
		ChangeFaceSize(want_size);

	return 1.f;
}

void FontFreeType::ChangeFaceSize(int want_size) const
{
	FT_Error error = FT_Set_Pixel_Sizes(face_, 0, want_size);
	if (error)
	{
		ASSERT2(0, "error set pixel size %d", error);
	}
	
	size_ = want_size;
	common_line_height_ = static_cast<int>(convert_fix26(face_->size->metrics.height) * 1.1f);
	common_base_ = static_cast<int>(convert_fix26(face_->size->metrics.ascender));
}

#endif // ERI_FONT_FREETYPE

#if defined(ERI_TEXTURE_READER_UIKIT) || defined(ERI_TEXTURE_READER_ANDROID)
#pragma mark FontSys

class FontSys : public Font
{
public:
	virtual bool Load(const std::string& path);

	virtual const Texture* CreateSpriteTxt(const std::string& tex_name,
		const TxtData& data,
		int font_size,
		int max_width,
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

const Texture* FontSys::CreateSpriteTxt(const std::string& tex_name,
		const TxtData& data,
		int font_size,
		int max_width,
		int& out_width,
		int& out_height) const
{
	ASSERT(!tex_name.empty());
	
	Root::Ins().texture_mgr()->ReleaseTexture(tex_name);

	Vector2 actual_size;
	
#ifdef ERI_TEXTURE_READER_UIKIT
	TextureReaderUIFont reader(data,
							   name_,
							   font_size,
							   max_width,
							   actual_size);
#elif defined(ERI_TEXTURE_READER_ANDROID)
	TextureReaderSysTxtAndroid reader(data.str, name_, font_size, data.is_pos_center, actual_size);
#else
	TextureReader reader(true);
#endif
	
	out_width = actual_size.x;
	out_height = actual_size.y;
	
	return Root::Ins().texture_mgr()->CreateTexture(tex_name, &reader);
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
			ASSERT2(0, "error done freetype %d", error);
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
					ASSERT2(0, "error init freetype %d", error);
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
