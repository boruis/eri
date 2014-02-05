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
  
struct TxtData
{
  TxtData() :
    is_pos_center(false),
    is_utf8(false),
    is_anti_alias(false)
  {
  }
  
  std::string str;
  bool is_pos_center;
  bool is_utf8;
  bool is_anti_alias;
};

int CreateUnicodeArray(const TxtData& data, uint32_t*& out_chars);
	
void CalculateTxtSize(const TxtData& data,
					  const Font* font,
					  int font_size,
					  float max_width,
					  float& out_width,
					  float& out_height);
	
void CalculateTxtSize(const uint32_t* chars,
					  int length,
					  const Font* font,
					  int font_size,
					  float max_width,
					  float& out_width,
					  float& out_height,
					  std::vector<float>* out_row_widths = NULL);

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
										   const TxtData& data,
										   int font_size,
										   int max_width,
										   int& out_width,
										   int& out_height) const { return NULL; }
	
	virtual float GetSizeScale(int want_size) const;
	
	void SetTextureFilter(TextureFilter filter_min, TextureFilter filter_mag) const;
	void SetTextureWrap(TextureWrap wrap_s, TextureWrap wrap_t) const;
	
	inline const Texture* texture() const { return texture_; }
	inline TextureFilter filter_min() const { return filter_min_; }
	inline TextureFilter filter_mag() const { return filter_mag_; }
	inline TextureWrap wrap_s() const { return wrap_s_; }
	inline TextureWrap wrap_t() const { return wrap_t_; }
	inline int size() const { return size_; }
	inline int common_line_height() const { return common_line_height_; }
	inline int common_base() const { return common_base_; }
	
	virtual bool is_atlas() const { return false; }

protected:
	const Texture*	texture_;
	
	mutable	TextureFilter	filter_min_;
	mutable	TextureFilter	filter_mag_;
	mutable TextureWrap wrap_s_;
	mutable TextureWrap wrap_t_;
	
	mutable	int	size_;
	mutable	int	common_line_height_;
	mutable	int	common_base_;
	
	std::map<uint32_t, CharSetting>	char_map_;
};

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
