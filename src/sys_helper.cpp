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
#include "android_helper.h"
#endif

#include <fstream>

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
	
void ReplaceBackslashToSlash(std::string& str)
{
	size_t length = str.length();
	for (int i = 0; i < length; ++i)
	{
		if (str[i] == '\\')
			str[i] = '/';
	}
}

bool GetFileContentString(const std::string& path, std::string& out_content)
{
	std::ifstream is;
	is.open(path.c_str(), std::ios::in);
	
	if (is.fail()) return false;
	
	// get length of file:
	is.seekg(0, std::ios::end);
	long long length = is.tellg();
	is.seekg(0, std::ios::beg);
	
	// allocate memory:
	char* buffer = new char[length + 1];
	
	// read data as a block:
	is.read(buffer, static_cast<std::streamsize>(length));
	is.close();
	
	buffer[length] = '\0';
	
	out_content = buffer;
	
	delete [] buffer;

	return true;
}
  
bool IsFileExist(const std::string& path)
{
	std::ifstream ifs;
	ifs.open(path.c_str(), std::ios::binary);
	
	if (ifs.fail())
		return false;
	
	ifs.close();
	
	return true;
}

FileReader::FileReader() : info_(NULL)
{
}

FileReader::~FileReader()
{
	Close();
}

#if ERI_PLATFORM == ERI_PLATFORM_ANDROID

struct FileReaderInfo
{
	AAsset* asset;
};
	
bool FileReader::Open(const char* path, bool is_binary)
{
	ASSERT(path);
	
	if (info_) AAsset_close(info_->asset);
	else info_ = new FileReaderInfo;

	info_->asset = AAssetManager_open(g_android_app->activity->assetManager, path, AASSET_MODE_STREAMING);
	
	if (info_->asset) return true;
	
	delete info_;
	info_ = NULL;
	
	return false;
}
	
bool FileReader::Close()
{
	if (info_)
	{
		AAsset_close(info_->asset);
		delete info_;
		info_ = NULL;
		
		return true;
	}
	
	return false;
}
	
size_t FileReader::Read(void* buf, size_t byte_count)
{
	ASSERT(buf && byte_count > 0 && info_);

	return AAsset_read(info_->asset, buf, byte_count);
}

#else

struct FileReaderInfo
{
	FILE* f;
};
	
bool FileReader::Open(const char* path, bool is_binary)
{
	ASSERT(path);
	
	if (info_) fclose(info_->f);
	else info_ = new FileReaderInfo;
	
	info_->f = fopen(path, is_binary ? "rb" : "r");
	
	if (info_->f) return true;
	
	delete info_;
	info_ = NULL;
	
	return false;
}
	
bool FileReader::Close()
{
	if (info_)
	{
		fclose(info_->f);
		delete info_;
		info_ = NULL;
		
		return true;
	}
	
	return false;
}
	
size_t FileReader::Read(void* buf, size_t byte_count)
{
	ASSERT(buf && byte_count > 0 && info_);
	
	return fread(buf, 1, byte_count, info_->f);
}

#endif

}
