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
#include "platform_helper.h"
#include "sys_helper.h"

namespace ERI {
	
#pragma mark TextureReaderUIImage

	TextureReaderUIImage::TextureReaderUIImage(const std::string& path,
											   bool generate_immediately)
		:
		TextureReader(generate_immediately),
		texture_data_(NULL)
	{
		NSString* absolute_path = [[NSString alloc] initWithUTF8String:GetAbsolutePath(path).c_str()];
		
		CGImageRef texture_image = [UIImage imageWithContentsOfFile:absolute_path].CGImage;
		
		[absolute_path release];
		
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
											 bool align_center,
											 Vector2& out_actual_size)
		:
		TextureReader(true)
	{
		NSString* font_str = [[NSString alloc] initWithUTF8String:font_name.c_str()];
		
		UIFont* font = [UIFont fontWithName:font_str size:font_size];
		
		[font_str release];
		
		if (font == nil)
		{
			NSLog(@"Failed to get font object");
			return;
		}

		NSString* txt_str = [[NSString alloc] initWithUTF8String:txt.c_str()];
		CGSize actual_size = [txt_str sizeWithFont:font constrainedToSize:CGSizeMake(1024.f, 1024.f)];
		
		width_ = next_power_of_2(actual_size.width);
		height_ = next_power_of_2(actual_size.height);
		
		void* data = calloc(height_, width_);
		
		CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceGray();
		CGContextRef context = CGBitmapContextCreate(data, width_, height_, 8, width_, colorSpace, kCGImageAlphaNone);
		CGColorSpaceRelease(colorSpace);
		
		CGContextSetGrayFillColor(context, 1.0, 1.0);
		CGContextTranslateCTM(context, 0.0, height_);
		CGContextScaleCTM(context, 1.0, -1.0); // NOTE: NSString draws in UIKit referential i.e. renders upside-down compared to CGBitmapContext referential
		UIGraphicsPushContext(context);
		
		[txt_str drawInRect:CGRectMake(0, 0, actual_size.width, actual_size.height)
				   withFont:font
			  lineBreakMode:UILineBreakModeWordWrap
				  alignment:align_center ? UITextAlignmentCenter : UITextAlignmentLeft];

		[txt_str release];
		UIGraphicsPopContext();
		CGContextRelease(context);
		
		out_actual_size.x = actual_size.width;
		out_actual_size.y = actual_size.height;

		texture_id_ = Root::Ins().renderer()->GenerateTexture(data, width_, height_, ALPHA);
		
		free(data);
	}
	
	TextureReaderUIFont::~TextureReaderUIFont()
	{
	}

}