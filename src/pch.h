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
#  define ERI_GL
#elif  ERI_PLATFORM == ERI_PLATFORM_IOS || ERI_PLATFORM == ERI_PLATFORM_ANDROID
#  define ERI_GLES
#endif

#if ERI_PLATFORM == ERI_PLATFORM_WIN || ERI_PLATFORM == ERI_PLATFORM_MAC
#  if !defined(ERI_TEXTURE_READER_NO_FREEIMAGE)
#    define ERI_TEXTURE_READER_FREEIMAGE
#  endif
#  if !defined(ERI_TEXTURE_READER_NO_LIBPNG)
#    define ERI_TEXTURE_READER_LIBPNG
#  endif
#elif ERI_PLATFORM == ERI_PLATFORM_IOS
#  if !defined(ERI_TEXTURE_READER_NO_UIKIT)
#    define ERI_TEXTURE_READER_UIKIT
#  endif
#  if !defined(ERI_TEXTURE_READER_NO_LIBPNG)
#    define ERI_TEXTURE_READER_LIBPNG
#  endif
#  if !defined(ERI_TEXTURE_READER_NO_PVR)
#    define ERI_TEXTURE_READER_PVR
#  endif
#elif ERI_PLATFORM == ERI_PLATFORM_ANDROID
#  if !defined(ERI_TEXTURE_READER_NO_LIBPNG)
#    define ERI_TEXTURE_READER_LIBPNG
#  endif
#  if !defined(ERI_TEXTURE_READER_NO_ANDROID)
#    define ERI_TEXTURE_READER_ANDROID
#  endif
#endif

//

#ifndef ERI_RENDERER_ES2
#define ERI_RENDERER_ES1
#endif

//

#if ERI_PLATFORM == ERI_PLATFORM_WIN
typedef unsigned int uint32_t;
#endif

//

#include <stddef.h>

// log

#ifdef DISABLE_LOG
#  define LOGI(...)
#  define LOGW(...)
#else
#  if ERI_PLATFORM == ERI_PLATFORM_ANDROID
#    include <android/log.h>
#    define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "eri", __VA_ARGS__))
#    define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "eri", __VA_ARGS__))
#  else
#    define LOGI(...) { fprintf(stdout, __VA_ARGS__); fputc('\n', stdout); }
#    define LOGW(...) { fprintf(stderr, __VA_ARGS__); fputc('\n', stderr); }
#  endif
#endif

// assert

#ifdef DISABLE_ASSERT
#  define ASSERT(exp)
#  define ASSERT2(exp, ...)
#else
#  include <assert.h>
#  define ASSERT(exp) { if (!(exp)) { LOGW("ASSERT failed: (%s) at %s:%d", #exp, __FILE__, __LINE__); assert(0);} }
#  define ASSERT2(exp, ...)	{ if (!(exp)) { LOGW("ASSERT failed: (%s) at %s:%d", #exp, __FILE__, __LINE__); LOGW(__VA_ARGS__); assert(0); } }
#endif

#endif // ERI_PCH_H
