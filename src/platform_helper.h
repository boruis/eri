/*
 *  platform_helper.h
 *  eri
 *
 *  Created by exe on 11/28/09.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#ifndef ERI_PLATFORM_HELPER_H
#define ERI_PLATFORM_HELPER_H

#include "renderer.h"

#include <string>

namespace ERI {

	const char* GetResourcePath();
	const char* GetHomePath();
	const char* GetStringFileContent(const std::string& file_path);
	
	void SetDeviceOrientation(ViewOrientation orientation);

}

#endif // ERI_PLATFORM_HELPER_H
