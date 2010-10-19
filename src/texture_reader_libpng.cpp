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
			fclose(f);
			return;
		}
		
		if (setjmp(png_jmpbuf(png_ptr)))
		{
			fclose(f);
			return;
		}
	
		png_init_io(png_ptr, f);
		png_set_sig_bytes(png_ptr, 8);
		
		png_read_info(png_ptr, info_ptr);
		
		width_ = info_ptr->width;
		height_ = info_ptr->height;
		
		ASSERT(info_ptr->bit_depth == 8);
		
		png_set_interlace_handling(png_ptr);
		png_read_update_info(png_ptr, info_ptr);
		
		/* read file */
		
		if (setjmp(png_jmpbuf(png_ptr)))
		{
			fclose(f);
			return;
		}
				
		png_bytep* row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height_);
				
		texture_data_ = malloc(width_ * height_ * 4);

		switch (info_ptr->color_type) {
			case PNG_COLOR_TYPE_RGB_ALPHA:
				{
					for (int y = 0; y < height_; y++)
						row_pointers[y] = &((png_byte*)texture_data_)[width_ * 4 * y];
					
					png_read_image(png_ptr, row_pointers);
				}
				break;
				
			case PNG_COLOR_TYPE_RGB:
				{
					for (int y = 0; y < height_; y++)
						row_pointers[y] = (png_byte*)malloc(width_ * 3);

					png_read_image(png_ptr, row_pointers);
										
					for (int y = 0; y < height_; y++) // copy RGB -> RGBA
					{
						for (int x = 0; x < width_; x++)
						{
							memcpy(&((png_byte*)texture_data_)[width_ * 4 * y + x * 4], &row_pointers[y][x * 3], 3);
							((png_byte*)texture_data_)[width_ * 4 * y + x * 4 + 3] = 0xFF; // alpha assign 255
						}
					}
					
					for (int y = 0; y < height_; y++)
						free(row_pointers[y]);
				}
				break;
			default:
				ASSERT(0);
				break;
		}

		free(row_pointers);
		
        fclose(f);
		
		texture_id_ = Root::Ins().renderer()->GenerateTexture(texture_data_, width_, height_, RGBA);
		
		/*
		
		//image format
		FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
		FIBITMAP* dib = NULL;
		
		//check the file signature and deduce its format
		fif = FreeImage_GetFileType(real_path.c_str(), 0);
		//if still unknown, try to guess the file format from the file extension
		if(fif == FIF_UNKNOWN) 
			fif = FreeImage_GetFIFFromFilename(real_path.c_str());
		//if still unkown, return failure
		if(fif == FIF_UNKNOWN)
			return;
		
		//check that the plugin has reading capabilities and load the file
		if(FreeImage_FIFSupportsReading(fif))
			dib = FreeImage_Load(fif, real_path.c_str());
		//if the image failed to load, return failure
		if(!dib)
			return;
		
		FIBITMAP* bitmap = FreeImage_ConvertTo32Bits(dib);
		FreeImage_Unload(dib);
		
		//get the image width and height
		width_ = FreeImage_GetWidth(bitmap);
		height_ = FreeImage_GetHeight(bitmap);
		
		int pixel_num = width_ * height_;
		
		//retrieve the image data
		texture_data_ = malloc(pixel_num * 4);
		BYTE* dst_buff = static_cast<BYTE*>(texture_data_);
		
		FreeImage_ConvertToRawBits(dst_buff, bitmap, FreeImage_GetPitch(bitmap), 32,
								   FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, TRUE);
		
		//BYTE* src_buff = FreeImage_GetBits(bitmap);
		//memcpy(dst_buff, src_buff, pixel_num * 4);
		
		BYTE swap_tmp;
		for (int i = 0; i < pixel_num; ++i)
		{
			swap_tmp = dst_buff[i * 4];
			dst_buff[i * 4] = dst_buff[i * 4 + 2];
			dst_buff[i * 4 + 2] = swap_tmp;
			
			//dst_buff[i * 4] = src_buff[i * 4 + 2];
			//dst_buff[i * 4 + 2] = src_buff[i * 4];
		}
		
		FreeImage_Unload(bitmap);
		
		texture_id_ = Root::Ins().renderer()->GenerateTexture(texture_data_, width_, height_, RGBA);
		 */
	}
	
	TextureReaderLibPNG::~TextureReaderLibPNG()
	{
		free(texture_data_);
	}
	
}
