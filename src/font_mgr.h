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

#ifdef ERI_FONT_FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

namespace ERI {

class Font;

int CreateUnicodeArray(const std::string& txt, bool is_utf8, uint32_t*& out_chars);
	
void CalculateTxtSize(const std::string& txt,
					  const Font* font,
					  int font_size,
					  bool is_utf8,
					  float& width,
					  float& height);
	
void CalculateTxtSize(const uint32_t* chars,
					  int length,
					  const Font* font,
					  int font_size,
					  float& width,
					  float& height,
					  std::vector<float>* row_widths = NULL);

#pragma mark Font
	
struct CharSetting
{
	int	x, y;
	int width, height;
	int x_offset, y_offset;
	int x_advance;
};

class Font
{
public:
	Font();
	virtual ~Font() {}

	virtual bool Load(const std::string& path) = 0;

	const CharSetting& GetCharSetting(uint32_t unicode) const;
	
	virtual const Texture* CreateSpriteTxt(const std::string& name,
										   const std::string& txt,
										   int size,
										   bool is_pos_center,
										   bool is_utf8,
										   bool is_anti_alias,
										   int& out_width,
										   int& out_height) const { return NULL; }
	
	virtual float GetSizeScale(int want_size) const;
	
	void SetTextureFilter(TextureFilter filter_min, TextureFilter filter_mag) const;
	
	inline const Texture* texture() const { return texture_; }
	inline TextureFilter filter_min() const { return filter_min_; }
	inline TextureFilter filter_mag() const { return filter_mag_; }
	inline int size() const { return size_; }
	inline int common_line_height() const { return common_line_height_; }
	inline int common_base() const { return common_base_; }
	
	virtual bool is_atlas() const { return false; }

protected:
	const Texture*	texture_;
	
	mutable	TextureFilter	filter_min_;
	mutable	TextureFilter	filter_mag_;
	
	int	size_;
	int	common_line_height_;
	int	common_base_;
	
	std::map<uint32_t, CharSetting>	char_map_;
};

#pragma mark FontFntScript

class FontFntScript : public Font
{
public:
	virtual bool Load(const std::string& path);

	virtual bool is_atlas() const { return true; }
};

#ifdef ERI_FONT_FREETYPE
#pragma mark FontFreeType

class FontFreeType : public Font
{
public:
	FontFreeType(FT_Library lib_ref, int pixel_height);
	virtual ~FontFreeType();
	
	virtual bool Load(const std::string& path);
	
	virtual const Texture* CreateSpriteTxt(const std::string& name,
										   const std::string& txt,
										   int size,
										   bool is_pos_center,
										   bool is_utf8,
										   bool is_anti_alias,
										   int& out_width,
										   int& out_height) const;

private:
	FT_Library	lib_ref_;
	FT_Face		face_;
};
#endif // ERI_FONT_FREETYPE

#ifdef ERI_TEXTURE_READER_UIKIT

class FontUIKit : public Font
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

#endif

#pragma mark FontMgr
	
class FontMgr
{
public:
	FontMgr();
	~FontMgr();
	
	const Font* GetFont(const std::string& path, int want_pixel_height = 32);
	
private:
	std::map<std::string, Font*>	font_map_;

#ifdef ERI_FONT_FREETYPE
	FT_Library ft_lib_;
#endif
};

}

#endif // ERI_FONT_MGR_H
