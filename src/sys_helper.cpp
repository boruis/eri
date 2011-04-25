//
//  sys_helper.cpp
//  eri
//
//  Created by exe on 4/22/11.
//  Copyright 2011 cobbler. All rights reserved.
//

#include "pch.h"

#include "sys_helper.h"

#if ERI_PLATFORM == ERI_PLATFORM_ANDROID
#include <jni.h>
#include <android/log.h>
extern JNIEnv*	global_env;
extern jobject	global_renderer;
extern jclass	global_renderer_cls;
#else
#include <fstream>
#endif

#include "platform_helper.h"

namespace ERI
{

std::string GetAbsolutePath(const std::string& path)
{
#if ERI_PLATFORM == ERI_PLATFORM_WIN
	if (path[1] != ':') return GetResourcePath() + std::string("\\") + path;
#else
	if (path[0] != '/') return GetResourcePath() + std::string("/") + path;
#endif
	
	return path;
}

std::string GetDir(const std::string& path)
{
	size_t pos = path.rfind('/');
	if (pos == std::string::npos)
		pos = path.rfind('\\');

	if (pos != std::string::npos)
		return path.substr(0, pos + 1);
	
	return std::string();
}

std::string GetFileName(const std::string& path)
{
	size_t pos = path.rfind('/');
	if (pos == std::string::npos)
		pos = path.rfind('\\');
	
	if (pos != std::string::npos)
		return path.substr(pos + 1);
	
	return path;
}

void SeperateDirFileName(const std::string& path,
						 std::string& out_dir, std::string& out_file_name)
{
	size_t pos = path.rfind('/');
	if (pos == std::string::npos)
		pos = path.rfind('\\');
	
	if (pos != std::string::npos)
	{
		out_dir = path.substr(0, pos + 1);
		out_file_name = path.substr(pos + 1);
	}
	else
	{
		out_dir.clear();
		out_file_name = path;
	}
}

std::string GetFileNameExtension(const std::string& file_name)
{
	size_t pos = file_name.rfind('.');
	if (pos != std::string::npos)
		return file_name.substr(pos + 1);
	
	return std::string();
}

std::string GetFileNameBase(const std::string& file_name)
{
	size_t pos = file_name.rfind('.');
	if (pos != std::string::npos)
		return file_name.substr(0, pos);
	
	return file_name;
}

void SeperateFileNameBaseExtension(const std::string& file_name,
								   std::string& out_base, std::string& out_extension)
{
	size_t pos = file_name.rfind('.');
	if (pos != std::string::npos)
	{
		out_base = file_name.substr(0, pos);
		out_extension = file_name.substr(pos + 1);
	}
	else
	{
		out_base = file_name;
		out_extension.clear();
	}
}

bool GetFileContentString(const std::string& path, std::string& out_content)
{
#if ERI_PLATFORM == ERI_PLATFORM_ANDROID
	jmethodID mid = global_env->GetMethodID(global_renderer_cls, "GetTxtFileContent", "(Ljava/lang/String;)Ljava/lang/String;");
	jstring s = static_cast<jstring>(global_env->CallObjectMethod(global_renderer, mid, global_env->NewStringUTF(path.c_str())));
	
	const char* cstr = global_env->GetStringUTFChars(s, 0);
	
	content = cstr;
	
	global_env->ReleaseStringUTFChars(s, cstr);
#else
	std::ifstream is;
	is.open(path.c_str(), std::ios::in);
	
	if (is.fail()) return false;
	
	// get length of file:
	is.seekg(0, std::ios::end);
	std::streampos length = is.tellg();
	is.seekg(0, std::ios::beg);
	
	// allocate memory:
	char* buffer = new char[length];
	
	// read data as a block:
	is.read(buffer, length);
	is.close();
	
	out_content = buffer;
	
	delete [] buffer;
#endif

	return true;
}

}
