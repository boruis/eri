
#ifndef ERI_TEXTURE_READER_ANDROID_H
#define ERI_TEXTURE_READER_ANDROID_H

#include "texture_reader.h"

#include <string>

namespace ERI {

	struct Vector2;

	class TextureReaderSysTxtAndroid : public TextureReader
	{
	public:
		TextureReaderSysTxtAndroid(const std::string& txt,
			const std::string& font_name,
			float font_size,
			bool align_center,
			Vector2& out_actual_size);
	};

}

#endif
