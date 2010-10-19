/*
 *  texture_reader_bitmap_factory.h
 *  eri
 *
 *  Created by exe on 01/27/10.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#ifndef ERI_TEXTURE_READER_BITMAP_FACTORY_H
#define ERI_TEXTURE_READER_BITMAP_FACTORY_H

#include "texture_reader.h"

#include <string>

namespace ERI {
	
	class TextureReaderBitmapFactory : public TextureReader
		{
		public:
			TextureReaderBitmapFactory(const std::string& path);
			virtual ~TextureReaderBitmapFactory();
		};
	
}

#endif // ERI_TEXTURE_READER_BITMAP_FACTORY_H
