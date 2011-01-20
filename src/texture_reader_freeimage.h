/*
 *  texture_reader_freeimage.h
 *  eri
 *
 *  Created by exe on 08/29/10.
 *  Copyright 2010 cobbler. All rights reserved.
 *
 */

#ifndef ERI_TEXTURE_READER_FREEIMAGE_H
#define ERI_TEXTURE_READER_FREEIMAGE_H

#include "texture_reader.h"

#include <string>

struct FIBITMAP;

namespace ERI {

	class TextureReaderFreeImage : public TextureReader
	{
	public:
		TextureReaderFreeImage(const std::string& path, bool generate_immediately);
		virtual ~TextureReaderFreeImage();

		virtual void Generate();
		
		virtual void* texture_data() { return texture_data_; }
		
	private:
		void* texture_data_;
	};

}

#endif // ERI_TEXTURE_READER_FREEIMAGE_H

