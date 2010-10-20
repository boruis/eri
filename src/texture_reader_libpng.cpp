/*
 *  texture_reader_libpng.cpp
 *  eri
 *
 *  Created by exe on 10/19/10.
 *  Copyright 2010 cobbler. All rights reserved.
 *
 */

#include "pch.h"

#include "texture_reader_libpng.h"

#include "png.h"

#include "root.h"
#include "renderer.h"
#include "platform_helper.h"

namespace ERI {
		
	TextureReaderLibPNG::TextureReaderLibPNG(const std::string& path) : texture_data_(NULL)
	{
		std::string real_path(path);
#if ERI_PLATFORM == ERI_PLATFORM_IOS || ERI_PLATFORM == ERI_PLATFORM_MAC
		if (path[0] != '/')
		{
			real_path = GetResourcePath() + std::string("/") + path;
		}
#endif
		
		FILE* f = fopen(real_path.c_str(), "rb");
		if (!f)
			return;
		
		png_byte header[8];
		fread(header, sizeof(png_byte), 8, f);
		if (png_sig_cmp(header, 0, 8) != 0)
		{
			fclose(f);
			return;
		}
		
		/* initialize stuff */
		
		png_structp png_ptr;
		png_infop info_ptr;
		
		png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (!png_ptr)
		{
			fclose(f);
			return;
		}
				
		info_ptr = png_create_info_struct(png_ptr);
		if (!info_ptr)
		{
			png_destroy_read_struct(&png_ptr, NULL, NULL);
			fclose(f);
			return;
		}
		
		if (setjmp(png_jmpbuf(png_ptr)))
		{
			png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
			fclose(f);
			return;
		}
	
		png_init_io(png_ptr, f);
		png_set_sig_bytes(png_ptr, 8);
		
		png_read_info(png_ptr, info_ptr);
		
		png_uint_32 w, h;
		int bit_depth, color_type, interlace_type, compression_type, filter_type;
		png_get_IHDR(png_ptr, info_ptr, &w, &h, &bit_depth, &color_type,
					 &interlace_type, &compression_type, &filter_type);
		
		width_ = w;
		height_ = h;
		
		ASSERT(bit_depth == 8);
		
		png_set_interlace_handling(png_ptr);

		if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY)
			png_set_add_alpha(png_ptr, 0xFF, PNG_FILLER_AFTER);
		
		png_read_update_info(png_ptr, info_ptr);
		png_get_IHDR(png_ptr, info_ptr, &w, &h, &bit_depth, &color_type,
					 &interlace_type, &compression_type, &filter_type);
		
		ASSERT(color_type == PNG_COLOR_TYPE_RGB_ALPHA);
		
		/* read file */
		
		if (setjmp(png_jmpbuf(png_ptr)))
		{
			png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
			fclose(f);
			return;
		}
				
		png_bytep* row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height_);
				
		texture_data_ = malloc(width_ * height_ * 4);

		for (int y = 0; y < height_; y++)
			row_pointers[y] = &((png_byte*)texture_data_)[width_ * 4 * y];
					
		png_read_image(png_ptr, row_pointers);

		free(row_pointers);
		
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		
        fclose(f);
		
		texture_id_ = Root::Ins().renderer()->GenerateTexture(texture_data_, width_, height_, RGBA);
	}
	
	TextureReaderLibPNG::~TextureReaderLibPNG()
	{
		free(texture_data_);
	}
	
}
