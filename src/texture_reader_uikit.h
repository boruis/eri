/*
 *  texture_reader_uikit.h
 *  eri
 *
 *  Created by exe on 11/30/09.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#ifndef ERI_TEXTURE_READER_UIIMAGE_H
#define ERI_TEXTURE_READER_UIIMAGE_H

#include "texture_reader.h"

#include <string>

namespace ERI {

	class TextureReaderUIImage : public TextureReader
	{
	public:
		TextureReaderUIImage(const std::string& path);
		virtual ~TextureReaderUIImage();
	};
	
	class TextureReaderUIFont : public TextureReader
	{
	public:
		TextureReaderUIFont(const std::string& txt, const std::string& font_name, float font_size, float w, float h);
		virtual ~TextureReaderUIFont();
	};

}

#endif // ERI_TEXTURE_READER_UIIMAGE_H