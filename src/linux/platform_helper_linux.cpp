/*
 *  platform_helper.cpp
 *  eri
 *
 *  Created by exe on 10/14/10.
 *  Copyright 2010 cobbler. All rights reserved.
 *
 */

#include "platform_helper.h"

#include <iconv.h>

#include <algorithm>
#include <fstream>

#include "rapidxml.hpp"

namespace ERI
{
	const char* GetResourcePath()
	{
		static const char* path = ".";
		return path;
	}

	const char* GetHomePath()
	{
		static const char* path = ".";
		return path;
	}

	int GetUnicodeFromUTF8(const std::string& str, int max_buff_length, uint32_t* buff)
	{
		iconv_t cd = iconv_open("UTF-32LE", "UTF-8");
		if ((iconv_t)-1 != cd)
		{
			char* in_start = const_cast<char*>(str.c_str());
			size_t in_left = str.length();
			char* out_start = (char*)buff;
			size_t out_left = max_buff_length * sizeof(uint32_t);
			iconv(cd, &in_start, &in_left, &out_start, &out_left);

			iconv_close(cd);

			return max_buff_length - out_left / sizeof(uint32_t);
		}

		LOGW("iconv open failed!");
		return 0;
	}

}
