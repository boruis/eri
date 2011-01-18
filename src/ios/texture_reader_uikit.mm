/*
 *  texture_reader_uikit.cpp
 *  eri
 *
 *  Created by exe on 11/30/09.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#include "texture_reader_uikit.h"
#include "root.h"
#include "renderer.h"

namespace ERI {
	
#pragma mark TextureReaderUIImage

	TextureReaderUIImage::TextureReaderUIImage(const std::string& path,
											   bool generate_immediately)
		:
		TextureReader(generate_immediately),
		texture_data_(NULL)
	{
		CGImageRef texture_image = [UIImage imageNamed:[NSString stringWithUTF8String:path.c_str()]].CGImage;
		
		if (texture_image == nil) {
			NSLog(@"Failed to load texture image");
			return;
		}

		width_ = CGImageGetWidth(texture_image);
		height_ = CGImageGetHeight(texture_image);
		
		texture_data_ = calloc(width_ * height_ * 4, sizeof(unsigned char));
		
		CGContextRef texture_context = CGBitmapContextCreate(texture_data_,
															 width_,
															 height_,
															 8, width_ * 4,
															 CGImageGetColorSpace(texture_image),
															 kCGImageAlphaPremultipliedLast);
		
        CGContextDrawImage(texture_context,
						   CGRectMake(0.0, 0.0, (float)width_, (float)height_),
						   texture_image);
		
        CGContextRelease(texture_context);
		
		if (generate_immediately)
		{
			Generate();
		}
	}
	
	TextureReaderUIImage::~TextureReaderUIImage()
	{
		free(texture_data_);
	}
	
	void TextureReaderUIImage::Generate()
	{
		texture_id_ = Root::Ins().renderer()->GenerateTexture(texture_data_, width_, height_, RGBA);
	}
	
#pragma mark TextureReaderUIFont
	
	TextureReaderUIFont::TextureReaderUIFont(const std::string& txt,
											 const std::string& font_name,
											 float font_size,
											 float w,
											 float h)
		:
		TextureReader(true)
	{
		NSUInteger			width, height, i;
		CGContextRef		context;
		void*				data;
		CGColorSpaceRef		colorSpace;
		UIFont*				font = nil;
		
		font = [UIFont fontWithName:[NSString stringWithUTF8String:font_name.c_str()] size:font_size];
		
		if (font == nil)
		{
			NSLog(@"Failed to get font object");
			return;
		}
		
		width = w;
		if((width != 1) && (width & (width - 1))) {
			i = 1;
			while(i < width) i *= 2;
			width = i;
		}
		height = h;
		if((height != 1) && (height & (height - 1))) {
			i = 1;
			while(i < height) i *= 2;
			height = i;
		}
		
		data = calloc(height, width);
		
		colorSpace = CGColorSpaceCreateDeviceGray();
		context = CGBitmapContextCreate(data, width, height, 8, width, colorSpace, kCGImageAlphaNone);
		CGColorSpaceRelease(colorSpace);
		
		CGContextSetGrayFillColor(context, 1.0, 1.0);
		CGContextTranslateCTM(context, 0.0, height);
		CGContextScaleCTM(context, 1.0, -1.0); //NOTE: NSString draws in UIKit referential i.e. renders upside-down compared to CGBitmapContext referential
		UIGraphicsPushContext(context);
		
		CGSize actualSize =	[[NSString stringWithUTF8String:txt.c_str()] drawInRect:CGRectMake(0, 0, w, h) withFont:font lineBreakMode:UILineBreakModeWordWrap alignment:UITextAlignmentLeft];
		UIGraphicsPopContext();
		CGContextRelease(context);
		
		width_ = width;
		height_ = height;
		texture_id_ = Root::Ins().renderer()->GenerateTexture(data, width_, height_, ALPHA);
		
		free(data);
	}
	
	TextureReaderUIFont::~TextureReaderUIFont()
	{
	}
	
}