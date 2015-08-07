/*
 *  platform_helper.cpp
 *  eri
 *
 *  Created by exe on 11/28/09.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#include "platform_helper.h"

namespace ERI {

	const char* GetResourcePath()
	{
		return [[[NSBundle mainBundle] resourcePath] UTF8String];
	}
	
	const char* GetHomePath()
	{
		return [NSHomeDirectory() UTF8String];
	}
	
	const char* GetBundlePath()
	{
		return [[[NSBundle mainBundle] bundlePath] UTF8String];
	}
	
	std::string GetWritePath()
	{
		return std::string(GetHomePath()) + "/Documents";
	}
	
	const char* GetStringFileContent(const std::string& file_path)
	{
		return [[NSString stringWithContentsOfFile:[NSString stringWithUTF8String:file_path.c_str()] encoding:NSUTF8StringEncoding error:nil] UTF8String];
	}
	
	int GetUnicodeFromUTF8(const std::string& str, int max_buff_length, uint32_t* buff)
	{
		NSString* utf8_str = [[NSString alloc] initWithUTF8String:str.c_str()];
		
		NSRange range;
		range.location = 0;
		range.length = [utf8_str length];
		NSUInteger used_length;
		
		[utf8_str getBytes:buff
				 maxLength:max_buff_length * sizeof(unsigned long)
				usedLength:&used_length
				  encoding:NSUTF32StringEncoding
				   options:NSStringEncodingConversionAllowLossy
					 range:range
			remainingRange:NULL];

#if !__has_feature(objc_arc)
		[utf8_str release];
#endif
		
		return static_cast<int>(used_length) / 4;
	}

	const char* GetLocale()
	{
		static char cstr[8];
		CFLocaleRef locale_ref = CFLocaleCopyCurrent();
		CFStringGetCString(CFLocaleGetIdentifier(locale_ref), cstr, 8, kCFStringEncodingASCII);
		CFRelease(locale_ref);

		return cstr;
	}
	
}
