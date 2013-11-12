#include "texture_reader_android.h"

#include "root.h"
#include "renderer.h"
#include "math_helper.h"
#include "android_helper.h"

namespace ERI {

	TextureReaderSysTxtAndroid::TextureReaderSysTxtAndroid(const std::string& txt,
		const std::string& font_name,
		float font_size,
		bool align_center,
		Vector2& out_actual_size)
		:
		TextureReader(true)
	{
		texture_id_ = Root::Ins().renderer()->GenerateTexture();

		mana::JavaCaller caller;

		caller.Set("com/exe/eri", "CreateSysTxtTexture", "(Landroid/app/Activity;Ljava/lang/String;Ljava/lang/String;FZ)I");

		jstring jstr_txt = caller.env->NewStringUTF(txt.c_str());
		jstring jstr_font_name = caller.env->NewStringUTF(font_name.c_str());
		int result = caller.env->CallStaticIntMethod(caller.user_class, caller.user_func, g_android_app->activity->clazz, jstr_txt, jstr_font_name, font_size, (jboolean)align_center);
		caller.env->DeleteLocalRef(jstr_txt);
		caller.env->DeleteLocalRef(jstr_font_name);

		width_ = result & 0xFFFF;
		height_ = (result >> 16) & 0xFFFF;

		out_actual_size.x = width_;
		out_actual_size.y = height_;

		width_ = next_power_of_2(width_);
		height_ = next_power_of_2(height_);

		caller.End();
	}

}
