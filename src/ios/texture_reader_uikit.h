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
	
	struct Vector2;

	class TextureReaderUIImage : public TextureReader
	{
	public:
		TextureReaderUIImage(const std::string& path, bool generate_immediately);
		virtual ~TextureReaderUIImage();
		
		virtual void Generate();
		
		virtual void* texture_data() { return texture_data_; }
		
	private:
		void* texture_data_;
	};
	
	class TextureReaderUIFont : public TextureReader
	{
	public:
		TextureReaderUIFont(const std::string& txt,
							const std::string& font_name,
							float font_size,
							bool align_center,
							float max_width,
							Vector2& out_actual_size);
		
		virtual ~TextureReaderUIFont();
	};

}

#endif // ERI_TEXTURE_READER_UIIMAGE_H