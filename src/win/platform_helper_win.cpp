/*
 *  platform_helper.cpp
 *  eri
 *
 *  Created by exe on 10/14/10.
 *  Copyright 2010 cobbler. All rights reserved.
 *
 */

#include "platform_helper.h"

#include <algorithm>
#include <fstream>
#include "rapidxml.hpp"

namespace ERI
{
	const char* GetResourcePath()
	{
		static char* path = ".";
		return path;
	}

	const char* GetHomePath()
	{
		static char* path = ".";
		return path;
	}

	int GetUnicodeFromUTF8(const std::string& str, int max_buff_length, uint32_t* buff)
	{
		// TODO: implement ...

		return 0;
	}

}
