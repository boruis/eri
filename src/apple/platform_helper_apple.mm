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
  
	std::string GetWritePath()
	{
		return std::string(GetHomePath()) + "/Documents";
	}
	
	const char* GetStringFileContent(const std::string& file_path)
	{
		return [[NSString stringWithContentsOfFile:[NSString stringWithUTF8String:file_path.c_str()] encoding:NSUTF8StringEncoding error:nil] UTF8String];
	}
		
	bool GetTextureAtlasArray(const std::string& name, TextureAtlasArray& out_array)
	{
		out_array.clear();
		
		std::string absolute_path(name);
		if (absolute_path[0] != '/')
			absolute_path = GetResourcePath() + std::string("/") + name;
		
		absolute_path += ".plist";
		
		NSString* path = [NSString stringWithUTF8String:absolute_path.c_str()];
		if (path == nil)
			return false;
		
		NSDictionary* dict = [[NSDictionary alloc] initWithContentsOfFile:path];
		if (dict == nil)
			return false;
		
		NSDictionary* frames = [dict objectForKey:@"frames"];
		if (frames == nil)
		{
#if !__has_feature(objc_arc)
			[dict release];
#endif
			return false;
		}

		NSDictionary* unit;
		TextureAtlasUnit data;
		
		NSMutableArray* keys = [[frames allKeys] mutableCopy];
		[keys sortUsingSelector:@selector(compare:)];

		NSInteger num = [keys count];
		for (int i = 0; i < num; ++i)
		{
			unit = [frames objectForKey:[keys objectAtIndex:i]];
			
			data.x = [[unit objectForKey:@"x"] intValue];
			data.y = [[unit objectForKey:@"y"] intValue];
			data.width = [[unit objectForKey:@"width"] intValue];
			data.height = [[unit objectForKey:@"height"] intValue];
			
			out_array.push_back(data);
		}
		
#if !__has_feature(objc_arc)
		[keys release];
		[dict release];
#endif
		
		return true;
	}

	bool GetTextureAtlasMap(const std::string& name, TextureAtlasMap& out_map)
	{
		out_map.clear();
		
		std::string absolute_path(name);
		if (absolute_path[0] != '/')
			absolute_path = GetResourcePath() + std::string("/") + name;
		
		absolute_path += ".plist";
		
		NSString* path = [NSString stringWithUTF8String:absolute_path.c_str()];
		if (path == nil)
			return false;
		
		NSDictionary* dict = [[NSDictionary alloc] initWithContentsOfFile:path];
		if (dict == nil)
			return false;
		
		NSDictionary* frames = [dict objectForKey:@"frames"];
		if (frames == nil)
		{
#if !__has_feature(objc_arc)
			[dict release];
#endif
			return false;
		}

		NSDictionary* unit;
		TextureAtlasUnit data;
		
		for (id key in frames)
		{
			unit = [frames objectForKey:key];
			
			data.x = [[unit objectForKey:@"x"] intValue];
			data.y = [[unit objectForKey:@"y"] intValue];
			data.width = [[unit objectForKey:@"width"] intValue];
			data.height = [[unit objectForKey:@"height"] intValue];
			
			out_map.insert(std::make_pair(std::string([key UTF8String]), data));
		}
		
#if !__has_feature(objc_arc)
		[dict release];
#endif
		
		return true;
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
