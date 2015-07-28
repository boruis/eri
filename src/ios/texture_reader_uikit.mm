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
#include "font_mgr.h"

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
		
#if !__has_feature(objc_arc)
		[absolute_path release];
#endif
		
		if (texture_image == nil) {
			NSLog(@"Failed to load texture image");
			return;
		}

		width_ = static_cast<int>(CGImageGetWidth(texture_image));
		height_ = static_cast<int>(CGImageGetHeight(texture_image));
		
		texture_data_ = calloc(width_ * height_ * 4, sizeof(unsigned char));
		
		CGContextRef texture_context = CGBitmapContextCreate(texture_data_,
															 width_,
															 height_,
															 8, width_ * 4,
															 CGImageGetColorSpace(texture_image),
															 kCGImageAlphaPremultipliedLast);
    
		alpha_premultiplied_ = true;
		
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
	
	TextureReaderUIFont::TextureReaderUIFont(const TxtData& data,
											 const std::string& font_name,
											 float font_size,
											 float max_width,
											 Vector2& out_actual_size)
		: TextureReader(true)
	{
		NSString* font_str = [[NSString alloc] initWithUTF8String:font_name.c_str()];
		UIFont* font = [UIFont fontWithName:font_str size:font_size];
		
#if !__has_feature(objc_arc)
		[font_str release];
#endif
		
		if (font == nil)
		{
			NSLog(@"Failed to get font object");
			return;
		}
		
		NSString* txt_str = [[NSString alloc] initWithUTF8String:data.str.c_str()];

		CGSize actual_size;
		NSLineBreakMode break_mode = NSLineBreakByWordWrapping;
		
		if (max_width > 0.f)
		{
			switch (data.line_break)
			{
				case LB_TRUNCATE_HEAD:
					break_mode = NSLineBreakByTruncatingHead;
					actual_size = [txt_str sizeWithFont:font forWidth:max_width lineBreakMode:break_mode];
					break;

				case LB_TRUNCATE_TAIL:
					break_mode = NSLineBreakByTruncatingTail;
					actual_size = [txt_str sizeWithFont:font forWidth:max_width lineBreakMode:break_mode];
					break;
					
				default:
					actual_size = [txt_str sizeWithFont:font constrainedToSize:CGSizeMake(max_width, 1024.f)];
					break;
			}
		}
		else
		{
			actual_size = [txt_str sizeWithFont:font constrainedToSize:CGSizeMake(1024.f, 1024.f)];
		}
		
		out_actual_size.x = actual_size.width;
		out_actual_size.y = actual_size.height;
		
		width_ = next_power_of_2(actual_size.width);
		height_ = next_power_of_2(actual_size.height);
		
		if (width_ == 0 || height_ == 0)
			return;

#ifdef ERI_RENDERER_ES2
		size_t bytes_per_row = width_ * 4;
		CGColorSpaceRef color_space = CGColorSpaceCreateDeviceRGB();
		CGBitmapInfo bitmap_info = kCGImageAlphaPremultipliedLast;
		alpha_premultiplied_ = true;
		PixelFormat pixel_format = RGBA;
#else
		size_t bytes_per_row = width_;
		CGColorSpaceRef color_space = CGColorSpaceCreateDeviceGray();
		CGBitmapInfo bitmap_info = kCGImageAlphaNone;
		PixelFormat pixel_format = ALPHA;
#endif
		
		void* texture_data = calloc(height_, bytes_per_row);
		
		CGContextRef context = CGBitmapContextCreate(texture_data, width_, height_, 8, bytes_per_row, color_space, bitmap_info);
		CGColorSpaceRelease(color_space);
		
		if (NULL == context)
		{
			NSLog(@"Failed to create bitmap context");
		}
		else
		{
#ifndef ERI_RENDERER_ES2
			CGContextSetGrayFillColor(context, 1.0, 1.0);
#endif
			
			CGContextTranslateCTM(context, 0.0, height_);
			CGContextScaleCTM(context, 1.0, -1.0); // NOTE: NSString draws in UIKit referential i.e. renders upside-down compared to CGBitmapContext referential
			UIGraphicsPushContext(context);
      
      if ([txt_str respondsToSelector:@selector(drawWithRect:options:attributes:context:)])
      {
        NSMutableParagraphStyle* style = [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
        style.alignment = data.is_pos_center ? NSTextAlignmentCenter : NSTextAlignmentLeft;
        style.lineBreakMode = break_mode;
        
        [txt_str drawInRect:CGRectMake(0, 0, actual_size.width, actual_size.height)
             withAttributes:@{
                              NSFontAttributeName: font,
                              NSParagraphStyleAttributeName: style,
                              NSForegroundColorAttributeName: [UIColor whiteColor]
                              }];
        
#if !__has_feature(objc_arc)
        [style release];
#endif
      }
      else
      {
        [txt_str drawInRect:CGRectMake(0, 0, actual_size.width, actual_size.height)
               withFont:font
            lineBreakMode:break_mode
              alignment:data.is_pos_center ? NSTextAlignmentCenter : NSTextAlignmentLeft];
      }
			
			UIGraphicsPopContext();
			CGContextRelease(context);
		}

#if !__has_feature(objc_arc)
		[txt_str release];
#endif
		
		texture_id_ = Root::Ins().renderer()->GenerateTexture(texture_data, width_, height_, pixel_format);
		
		free(texture_data);
	}
	
	TextureReaderUIFont::~TextureReaderUIFont()
	{
	}

}