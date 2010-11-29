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
	
	const char* GetStringFileContent(const std::string& file_path)
	{
		return [[NSString stringWithContentsOfFile:[NSString stringWithUTF8String:file_path.c_str()] encoding:NSUTF8StringEncoding error:nil] UTF8String];
	}
		
	bool GetTextureAtlasArray(const std::string& name, TextureAtlasArray& out_array)
	{
		out_array.clear();
		
		std::string absolute_path(name);
#if ERI_PLATFORM == ERI_PLATFORM_IOS || ERI_PLATFORM == ERI_PLATFORM_MAC
		if (absolute_path[0] != '/')
		{
			absolute_path = GetResourcePath() + std::string("/") + name;
		}
#endif
		absolute_path += ".plist";
		
		NSString* path = [NSString stringWithUTF8String:absolute_path.c_str()];
		if (path == nil)
			return false;
		
		NSDictionary* dict = [[NSDictionary alloc] initWithContentsOfFile:path];
		if (dict == nil)
			return false;
		
		NSDictionary* frames = [dict objectForKey:@"frames"];
		if (frames == nil)
			return false;

		NSDictionary* unit;
		TextureAtlasUnit data;
		
		NSMutableArray* keys = [[frames allKeys] mutableCopy];
		[keys sortUsingSelector:@selector(compare:)];

		int num = [keys count];
		for (int i = 0; i < num; ++i)
		{
			unit = [frames objectForKey:[keys objectAtIndex:i]];
			
			data.x = [[unit objectForKey:@"x"] intValue];
			data.y = [[unit objectForKey:@"y"] intValue];
			data.width = [[unit objectForKey:@"width"] intValue];
			data.height = [[unit objectForKey:@"height"] intValue];
			
			out_array.push_back(data);
		}
		
		[keys release];
		[dict release];
		
		return true;
	}

	bool GetTextureAtlasMap(const std::string& name, TextureAtlasMap& out_map)
	{
		out_map.clear();
		
		std::string absolute_path(name);
#if ERI_PLATFORM == ERI_PLATFORM_IOS || ERI_PLATFORM == ERI_PLATFORM_MAC
		if (absolute_path[0] != '/')
		{
			absolute_path = GetResourcePath() + std::string("/") + name;
		}
#endif
		absolute_path += ".plist";
		
		NSString* path = [NSString stringWithUTF8String:absolute_path.c_str()];
		if (path == nil)
			return false;
		
		NSDictionary* dict = [[NSDictionary alloc] initWithContentsOfFile:path];
		if (dict == nil)
			return false;
		
		NSDictionary* frames = [dict objectForKey:@"frames"];
		if (frames == nil)
			return false;

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
		
		[dict release];
		
		return true;
	}
	
}
