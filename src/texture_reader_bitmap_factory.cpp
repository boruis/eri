/*
 *  texture_reader_bitmap_factory.cpp
 *  eri
 *
 *  Created by exe on 01/27/10.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#include "texture_reader_bitmap_factory.h"
#include "root.h"
#include "renderer.h"

#include <jni.h>
#include <android/log.h>

extern JNIEnv*	global_env;
extern jobject	global_renderer;
extern jclass	global_renderer_cls;

namespace ERI {
	
	TextureReaderBitmapFactory::TextureReaderBitmapFactory(const std::string& path)
	{
		texture_id_ = Root::Ins().renderer()->BindTexture();

		// TODO: performance optimize
		jmethodID mid = global_env->GetMethodID(global_renderer_cls, "CreateTexture", "(Ljava/lang/String;)I");
		int ret = static_cast<int>(global_env->CallIntMethod(global_renderer, mid, global_env->NewStringUTF(path.c_str())));

		width_ = ret / 10000;
		height_ = ret % 10000;
		
		__android_log_print(ANDROID_LOG_INFO, "ERI", "TextureReaderBitmapFactory %s w = %d h = %d", path.c_str(), width_, height_);
	}
	
	TextureReaderBitmapFactory::~TextureReaderBitmapFactory()
	{
	}
	
}

