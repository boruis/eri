//
//  sys_helper.h
//  eri
//
//  Created by exe on 4/22/11.
//  Copyright 2011 cobbler. All rights reserved.
//

#ifndef ERI_SYS_HELPER_H
#define ERI_SYS_HELPER_H

#include <string>

namespace ERI
{

std::string GetAbsolutePath(const std::string& path);

std::string GetDir(const std::string& path);
std::string GetFileName(const std::string& path);
void SeperateDirFileName(const std::string& path,
						 std::string& out_dir, std::string& out_file_name);

std::string GetFileNameExtension(const std::string& file_name);
std::string GetFileNameBase(const std::string& file_name);
void SeperateFileNameBaseExtension(const std::string& file_name,
								   std::string& out_base, std::string& out_extension);
	
bool GetFileContentString(const std::string& path, std::string& out_content);
	
}

#endif // ERI_SYS_HELPER_H