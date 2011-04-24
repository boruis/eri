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
										   uint32_t* unicodes,
										   int length,
										   bool is_anti_alias,
										   int& out_width,
										   int& out_height) const { return NULL; }
	
	inline const Texture* texture() const { return texture_; }
	inline TextureFilter filter_min() const { return filter_min_; }
	inline TextureFilter filter_mag() const { return filter_mag_; }
	inline int size() const { return size_; }
	inline int common_line_height() const { return common_line_height_; }
	inline int common_base() const { return common_base_; }
	
	virtual bool is_atlas() const { return true; }

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
										   uint32_t* unicodes,
										   int length,
										   bool is_anti_alias,
										   int& out_width,
										   int& out_height) const;
	
	virtual bool is_atlas() const { return false; }
	
private:
	FT_Library	lib_ref_;
	FT_Face		face_;
};
#endif // ERI_FONT_FREETYPE

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
