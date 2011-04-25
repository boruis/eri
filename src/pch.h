#ifndef ERI_PCH_H
#define ERI_PCH_H

// platform

#define ERI_PLATFORM_UNKNOW   0
#define ERI_PLATFORM_WIN      1
#define ERI_PLATFORM_MAC      2
#define ERI_PLATFORM_IOS      3
#define ERI_PLATFORM_ANDROID  4

#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32)
#  define ERI_PLATFORM ERI_PLATFORM_WIN
#elif defined(__APPLE_CC__)
#  if __ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__ >= 30000 || __IPHONE_OS_VERSION_MIN_REQUIRED >= 30000
#    define ERI_PLATFORM ERI_PLATFORM_IOS
#  else
#    define ERI_PLATFORM ERI_PLATFORM_MAC
#  endif
#elif defined(OS_ANDROID)
#	 define ERI_PLATFORM ERI_PLATFORM_ANDROID
#else
#  define ERI_PLATFORM ERI_PLATFORM_UNKNOW
#endif

#ifdef __APPLE__
#include "TargetConditionals.h"
#endif

// config

#if ERI_PLATFORM == ERI_PLATFORM_WIN || ERI_PLATFORM == ERI_PLATFORM_MAC
#  if !defined(ERI_TEXTURE_READER_NO_FREEIMAGE)
#    define ERI_TEXTURE_READER_FREEIMAGE
#  else
#    define ERI_TEXTURE_READER_LIBPNG
#  endif
#elif ERI_PLATFORM == ERI_PLATFORM_IOS
#  if !defined(ERI_TEXTURE_READER_NO_UIKIT)
#    define ERI_TEXTURE_READER_UIKIT
#  else
#    define ERI_TEXTURE_READER_LIBPNG
#  endif
#  if !defined(ERI_TEXTURE_READER_NO_PVR)
#    define ERI_TEXTURE_READER_PVR
#  endif
#elif ERI_PLATFORM == ERI_PLATFORM_ANDROID
#  define ERI_TEXTURE_READER_BITMAP_FACTORY
#endif

//

#if ERI_PLATFORM == ERI_PLATFORM_WIN
typedef unsigned int uint32_t;
#endif

//

#include <stddef.h>

// assert

#include <assert.h>
#define ASSERT(exp)								assert(exp)
#define ASSERT2(exp, description)				assert(exp)
#define ASSERT3(exp, description_format, ...)	assert(exp)

#endif // ERI_PCH_H
