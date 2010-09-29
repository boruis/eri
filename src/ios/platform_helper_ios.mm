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
	
	void SetDeviceOrientation(ViewOrientation orientation)
	{
		switch (orientation) {
			case PORTRAIT_HOME_BOTTOM:
				[[UIApplication sharedApplication] setStatusBarOrientation: UIInterfaceOrientationPortrait animated:NO];
				break;
			case PORTRAIT_HOME_TOP:
				[[UIApplication sharedApplication] setStatusBarOrientation: UIInterfaceOrientationPortraitUpsideDown animated:NO];
				break;
			case LANDSCAPE_HOME_RIGHT:
				[[UIApplication sharedApplication] setStatusBarOrientation: UIInterfaceOrientationLandscapeRight animated:NO];
				break;
			case LANDSCAPE_HOME_LEFT:
				[[UIApplication sharedApplication] setStatusBarOrientation: UIInterfaceOrientationLandscapeLeft animated:NO];
				break;
			default:
				ASSERT(0);
				break;
		}
	}
	
	void GetTextureAtlasArray(const std::string& name, TextureAtlasArray& out_array)
	{
		out_array.clear();
		
		NSString* path = [[NSBundle mainBundle] pathForResource:[NSString stringWithUTF8String:name.c_str()] ofType:@"plist"];
		NSDictionary* dict = [[NSDictionary alloc] initWithContentsOfFile:path];
		NSDictionary* frames = [dict objectForKey:@"frames"];

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
	}

	void GetTextureAtlasMap(const std::string& name, TextureAtlasMap& out_map)
	{
		out_map.clear();
		
		NSString* path = [[NSBundle mainBundle] pathForResource:[NSString stringWithUTF8String:name.c_str()] ofType:@"plist"];
		NSDictionary* dict = [[NSDictionary alloc] initWithContentsOfFile:path];
		NSDictionary* frames = [dict objectForKey:@"frames"];

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
	}
	
}
