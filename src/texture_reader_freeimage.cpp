/*
 *  texture_reader_freeimage.cpp
 *  eri
 *
 *  Created by exe on 08/29/10.
 *  Copyright 2010 cobbler. All rights reserved.
 *
 */

#include "pch.h"

#include "texture_reader_freeimage.h"

#include "FreeImage.h"

#include "root.h"
#include "renderer.h"
#include "platform_helper.h"

namespace ERI {
	
#pragma mark TextureReaderFreeImage

	TextureReaderFreeImage::TextureReaderFreeImage(const std::string& path) : texture_data_(NULL)
	{
		std::string real_path(path);
#if ERI_PLATFORM == ERI_PLATFORM_IOS || ERI_PLATFORM == ERI_PLATFORM_MAC
		if (path[0] != '/')
		{
			real_path = GetResourcePath() + std::string("/") + path;
		}
#endif
		
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
	}
	
	TextureReaderFreeImage::~TextureReaderFreeImage()
	{
		free(texture_data_);
	}
	
}
