/*
 *  texture_reader_pvr.cpp
 *  eri
 *
 *  Created by exe on 2011/3/11.
 *  Copyright 2011 cobbler. All rights reserved.
 *
 */

#include "texture_reader_pvr.h"

#include "root.h"
#include "platform_helper.h"

#define PVR_TEXTURE_FLAG_TYPE_MASK	0xff

static char gPVRTexIdentifier[5] = "PVR!";

enum
{
	kPVRTextureFlagTypePVRTC_2 = 24,
	kPVRTextureFlagTypePVRTC_4
};

typedef struct _PVRTexHeader
{
	uint32_t headerLength;
	uint32_t height;
	uint32_t width;
	uint32_t numMipmaps;
	uint32_t flags;
	uint32_t dataLength;
	uint32_t bpp;
	uint32_t bitmaskRed;
	uint32_t bitmaskGreen;
	uint32_t bitmaskBlue;
	uint32_t bitmaskAlpha;
	uint32_t pvrTag;
	uint32_t numSurfs;
} PVRTexHeader;

namespace ERI
{

TextureReaderPVR::TextureReaderPVR(const std::string& path, bool generate_immediately)
	:	TextureReader(generate_immediately),
		internal_format_(RGBA_PVR_4BPP),
		has_alpha_(false),
		load_success_(false)
{
	image_data_ = nil;
	
	std::string real_path = GetResourcePath();
	real_path += "/";
	real_path += path;
	
	data_ = [[NSData alloc] initWithContentsOfFile:[NSString stringWithUTF8String:real_path.c_str()]];
	if (data_)
	{
		image_data_ = [[NSMutableArray alloc] initWithCapacity:10];
		
		load_success_ = UnpackPVRData();
		
		[data_ release];
		
		if (load_success_ && generate_immediately)
		{
			Generate();
		}
	}
}
	
TextureReaderPVR::~TextureReaderPVR()
{
	[image_data_ release];
}

bool TextureReaderPVR::UnpackPVRData()
{
	bool success = false;
	
	PVRTexHeader *header = NULL;
	uint32_t flags, pvrTag;
	uint32_t dataLength = 0, dataOffset = 0, dataSize = 0;
	uint32_t blockSize = 0, widthBlocks = 0, heightBlocks = 0;
	uint32_t w = 0, h = 0, bpp = 4;
	uint8_t *bytes = NULL;
	uint32_t formatFlags;
	
	header = (PVRTexHeader *)[data_ bytes];
	
	pvrTag = CFSwapInt32LittleToHost(header->pvrTag);
	
	if (gPVRTexIdentifier[0] != ((pvrTag >>  0) & 0xff) ||
			gPVRTexIdentifier[1] != ((pvrTag >>  8) & 0xff) ||
			gPVRTexIdentifier[2] != ((pvrTag >> 16) & 0xff) ||
			gPVRTexIdentifier[3] != ((pvrTag >> 24) & 0xff))
	{
		return false;
	}
	
	flags = CFSwapInt32LittleToHost(header->flags);
	formatFlags = flags & PVR_TEXTURE_FLAG_TYPE_MASK;
	
	if (formatFlags == kPVRTextureFlagTypePVRTC_4 || formatFlags == kPVRTextureFlagTypePVRTC_2)
	{
		[image_data_ removeAllObjects];
		
		if (formatFlags == kPVRTextureFlagTypePVRTC_4)
			internal_format_ = RGBA_PVR_4BPP;
		else if (formatFlags == kPVRTextureFlagTypePVRTC_2)
			internal_format_ = RGBA_PVR_2BPP;
		
		width_ = w = CFSwapInt32LittleToHost(header->width);
		height_ = h = CFSwapInt32LittleToHost(header->height);
		
		if (CFSwapInt32LittleToHost(header->bitmaskAlpha))
			has_alpha_ = true;
		else
			has_alpha_ = false;
		
		dataLength = CFSwapInt32LittleToHost(header->dataLength);
		
		bytes = ((uint8_t *)[data_ bytes]) + sizeof(PVRTexHeader);
		
		// Calculate the data size for each texture level and respect the minimum number of blocks
		while (dataOffset < dataLength)
		{
			if (formatFlags == kPVRTextureFlagTypePVRTC_4)
			{
				blockSize = 4 * 4; // Pixel by pixel block size for 4bpp
				widthBlocks = w / 4;
				heightBlocks = h / 4;
				bpp = 4;
			}
			else
			{
				blockSize = 8 * 4; // Pixel by pixel block size for 2bpp
				widthBlocks = w / 8;
				heightBlocks = h / 4;
				bpp = 2;
			}
			
			// Clamp to minimum number of blocks
			if (widthBlocks < 2)
				widthBlocks = 2;
			if (heightBlocks < 2)
				heightBlocks = 2;
			
			dataSize = widthBlocks * heightBlocks * ((blockSize  * bpp) / 8);
			
			[image_data_ addObject:[NSData dataWithBytes:bytes+dataOffset length:dataSize]];
			
			dataOffset += dataSize;
			
			w = MAX(w >> 1, 1);
			h = MAX(h >> 1, 1);
		}
		
		success = true;
	}
	
	return success;
}

void TextureReaderPVR::Generate()
{
	if (load_success_)
	{
		NSData* data = [image_data_ objectAtIndex:0];
		
		texture_id_ = Root::Ins().renderer()->GenerateTexture([data bytes], width_, height_, internal_format_, [data length]);
		
		// TODO: mipmap handle
		
//	if ([_imageData count] > 1)
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//	else
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	
//	for (int i=0; i < [_imageData count]; i++)
//	{
//		data = [_imageData objectAtIndex:i];
//		glCompressedTexImage2D(GL_TEXTURE_2D, i, _internalFormat, width, height, 0, [data length], [data bytes]);
//		
//		err = glGetError();
//		if (err != GL_NO_ERROR)
//		{
//			NSLog(@"Error uploading compressed texture level: %d. glError: 0x%04X", i, err);
//			return FALSE;
//		}
//		
//		width = MAX(width >> 1, 1);
//		height = MAX(height >> 1, 1);
//	}
		
		[image_data_ removeAllObjects];
	}
}

}
