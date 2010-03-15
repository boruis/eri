/*
 *  texture_reader.h
 *  eri
 *
 *  Created by exe on 11/30/09.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#ifndef ERI_TEXTURE_READER_H
#define ERI_TEXTURE_READER_H

namespace ERI {

	class TextureReader
	{
	public:
		TextureReader() : width_(-1), height_(-1), texture_id_(0) {}
		virtual ~TextureReader() {}
		
		inline int width() { return width_; }
		inline int height() { return height_; }
		
		inline unsigned int texture_id() { return texture_id_; }
		
	protected:
		int			width_;
		int			height_;
		
		unsigned int	texture_id_;
	};
}

#endif // ERI_TEXTURE_READER_H
