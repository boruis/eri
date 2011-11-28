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
		
	TextureReaderLibPNG::TextureReaderLibPNG(const std::string& path,
											 bool generate_immediately)
		:
		TextureReader(generate_immediately),
		texture_data_(NULL)
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
		
		if (!Root::Ins().renderer()->caps().is_support_non_power_of_2_texture)
		{
			if (!is_power_of_2(width_))
			{
				int new_width = prev_power_of_2(width_);

				void* new_texture_data = malloc(new_width * height_ * 4);
				memset(new_texture_data, 255, new_width * height_ * 4);
				unsigned char* src_data = static_cast<unsigned char*>(texture_data_);
				unsigned char* dst_data = static_cast<unsigned char*>(new_texture_data);
				
				float col_mapping, ratio;
				int begin_idx;
				unsigned char *begin_texel, *end_texel, *result_texel;
				for (int row = 0; row < height_; ++row)
				{
					for (int col = 0; col < new_width; ++col)
					{
						col_mapping = static_cast<float>(col) / (new_width - 1) * (width_ - 1);
						begin_idx = static_cast<int>(col_mapping);
						
						ratio = col_mapping - begin_idx;
						if (ratio > 0.0f)
						{
							begin_texel = src_data + (row * width_ + begin_idx) * 4;
							end_texel = src_data + (row * width_ + begin_idx + 1) * 4;
							result_texel = dst_data + (row * new_width + col) * 4;
							
							result_texel[0] = static_cast<unsigned char>(begin_texel[0] * (1 - ratio) + end_texel[0] * ratio);
							result_texel[1] = static_cast<unsigned char>(begin_texel[1] * (1 - ratio) + end_texel[1] * ratio);
							result_texel[2] = static_cast<unsigned char>(begin_texel[2] * (1 - ratio) + end_texel[2] * ratio);
							result_texel[3] = static_cast<unsigned char>(begin_texel[3] * (1 - ratio) + end_texel[3] * ratio);
						}
						else
						{
							memcpy(dst_data + (row * new_width + col) * 4,
								   src_data + (row * width_ + begin_idx) * 4, 4);
						}
					}
				}
							
				free(texture_data_);
				texture_data_ = new_texture_data;
				
				printf("non power of 2 texture width %d -> %d\n", width_, new_width);
				width_ = new_width;
			}

			if (!is_power_of_2(height_))
			{
				int new_height = prev_power_of_2(height_);
				
				void* new_texture_data = malloc(width_ * new_height * 4);
				memset(new_texture_data, 255, width_ * new_height * 4);
				unsigned char* src_data = static_cast<unsigned char*>(texture_data_);
				unsigned char* dst_data = static_cast<unsigned char*>(new_texture_data);
				
				float row_mapping, ratio;
				int begin_idx;
				unsigned char *begin_texel, *end_texel, *result_texel;
				for (int row = 0; row < new_height; ++row)
				{
					for (int col = 0; col < width_; ++col)
					{
						row_mapping = static_cast<float>(row) / (new_height - 1) * (height_ - 1);
						begin_idx = static_cast<int>(row_mapping);
						
						ratio = row_mapping - begin_idx;
						if (ratio > 0.0f)
						{
							begin_texel = src_data + (begin_idx * width_ + col) * 4;
							end_texel = src_data + ((begin_idx + 1) * width_ + col) * 4;
							result_texel = dst_data + (row * width_ + col) * 4;
							
							result_texel[0] = static_cast<unsigned char>(begin_texel[0] * (1 - ratio) + end_texel[0] * ratio);
							result_texel[1] = static_cast<unsigned char>(begin_texel[1] * (1 - ratio) + end_texel[1] * ratio);
							result_texel[2] = static_cast<unsigned char>(begin_texel[2] * (1 - ratio) + end_texel[2] * ratio);
							result_texel[3] = static_cast<unsigned char>(begin_texel[3] * (1 - ratio) + end_texel[3] * ratio);
						}
						else
						{
							memcpy(dst_data + (row * width_ + col) * 4,
								   src_data + (begin_idx * width_ + col) * 4, 4);
						}
					}
				}
							
				free(texture_data_);
				texture_data_ = new_texture_data;
				
				printf("non power of 2 texture height %d -> %d\n", height_, new_height);
				height_ = new_height;
			}
		}

		if (generate_immediately)
		{
			Generate();
		}
	}
	
	TextureReaderLibPNG::~TextureReaderLibPNG()
	{
		free(texture_data_);
	}
	
	void TextureReaderLibPNG::Generate()
	{
		texture_id_ = Root::Ins().renderer()->GenerateTexture(texture_data_, width_, height_, RGBA);
	}
}
