#include "platform_helper.h"

#include <fstream>
#include <algorithm>

#include "android_helper.h"

#include "rapidxml.hpp"

namespace ERI
{
	static const char* GetInternalPath()
	{
		if (g_android_app->activity->internalDataPath)
			return g_android_app->activity->internalDataPath;

		static std::string path;

		if (path.empty())
		{
			mana::JavaCaller caller;
			caller.Set("com/exe/eri", "GetInternalPath", "(Landroid/app/Activity;)Ljava/lang/String;");
			jstring jpath = (jstring)caller.env->CallStaticObjectMethod(caller.user_class, caller.user_func, g_android_app->activity->clazz);
			const char* cpath = caller.env->GetStringUTFChars(jpath, NULL);
			if (cpath)
			{
				path = cpath;
				caller.env->ReleaseStringUTFChars(jpath, cpath);

				LOGI("internal -> %s", path.c_str());
			}
			caller.End();
		}

		return path.c_str();
	}

	const char* GetResourcePath()
	{
		return GetInternalPath();
	}

	const char* GetHomePath()
	{
		return GetInternalPath();
	}

	std::string GetWritePath()
	{
		return GetInternalPath();
	}

	int GetUnicodeFromUTF8(const std::string& str, int max_buff_length, uint32_t* buff)
	{
		// TODO: implement ...

		return 0;
	}

	const char* GetLocale()
	{
		static std::string locale;

		mana::JavaCaller caller;
		caller.Set("com/exe/eri", "GetLocale", "()Ljava/lang/String;");
		jstring jstr_locale = (jstring)caller.env->CallStaticObjectMethod(caller.user_class, caller.user_func);
		const char* str_locale = caller.env->GetStringUTFChars(jstr_locale, NULL);
		if (str_locale)
		{
			locale = str_locale;
			caller.env->ReleaseStringUTFChars(jstr_locale, str_locale);
		}
		caller.End();

		return locale.c_str();
	}

}
