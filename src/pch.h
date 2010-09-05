#ifndef ERI_PCH_H
#define ERI_PCH_H

// platform

#define ERI_PLATFORM_UNKNOW		0
#define ERI_PLATFORM_WIN		1
#define ERI_PLATFORM_MAC		2
#define ERI_PLATFORM_IOS		3
#define ERI_PLATFORM_ANDROID	4

#if defined( __WIN32__ ) || defined( _WIN32 ) || defined( WIN32 )
#   define ERI_PLATFORM ERI_PLATFORM_WIN
#elif defined( __APPLE_CC__ )
#   if __ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__ >= 30000 || __IPHONE_OS_VERSION_MIN_REQUIRED >= 30000
#       define ERI_PLATFORM ERI_PLATFORM_IOS
#   else
#       define ERI_PLATFORM ERI_PLATFORM_MAC
#   endif
#elif defined( OS_ANDROID )
#	define ERI_PLATFORM ERI_PLATFORM_ANDROID
#else
#   define ERI_PLATFORM ERI_PLATFORM_UNKNOW
#endif

//

#include <cstddef>

// assert

#include <cassert>
#define ASSERT(exp)					assert(exp)
#define ASSERT2(exp, description)	assert(exp)

#endif // ERI_PCH_H
