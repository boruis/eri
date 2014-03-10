/*
 *  texture_reader_pvr.h
 *  eri
 *
 *  Created by exe on 2011/3/11.
 *  Copyright 2011 cobbler. All rights reserved.
 *
 */

#ifndef ERI_TEXTURE_READER_PVR_H
#define ERI_TEXTURE_READER_PVR_H

#include "pch.h"

#ifdef ERI_TEXTURE_READER_PVR

#include "texture_reader.h"

#include <string>

#include "renderer.h"

#ifdef __OBJC__
@class NSData;
@class NSMutableArray;
#endif

namespace ERI
{
	
class TextureReaderPVR : public TextureReader
{
public:
	TextureReaderPVR(const std::string& path, bool generate_immediately);
	virtual ~TextureReaderPVR();
	
	virtual void Generate();
	
private:
	bool UnpackPVRData();
	
	PixelFormat internal_format_;
	bool has_alpha_;
	
	bool load_success_;
	
#ifdef __OBJC__
	NSData*					data_;
	NSMutableArray*	image_data_;
#endif
};

}

#endif // ERI_TEXTURE_READER_PVR

#endif // ERI_TEXTURE_READER_PVR_H
