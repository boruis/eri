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
			JavaCaller caller;
			caller.Set(g_class_name, "GetInternalPath", "(Landroid/app/Activity;)Ljava/lang/String;");
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

	bool GetTextureAtlasArray(const std::string& name, TextureAtlasArray& out_array)
	{
		TextureAtlasMap tmp_map;
		if (GetTextureAtlasMap(name, tmp_map))
		{
			std::vector<std::string> for_sort_array;

			TextureAtlasMap::iterator it = tmp_map.begin();
			for (; it != tmp_map.end(); ++it)
			{
				for_sort_array.push_back(it->first);
			}

			if (for_sort_array.empty())
				return false;

			std::sort(for_sort_array.begin(), for_sort_array.end());

			out_array.clear();

			int unit_num = for_sort_array.size();
			for (int i = 0; i < unit_num; ++i)
			{
				out_array.push_back(tmp_map[for_sort_array[i]]);
			}

			return true;
		}

		return false;
	}

	bool GetTextureAtlasMap(const std::string& name, TextureAtlasMap& out_map)
	{
		std::string path = name + ".plist";

		if (path[0] != '/')
			path = GetResourcePath() + std::string("/") + path;

		std::ifstream ifs;
		ifs.open(path.c_str(), std::ios::binary);

		if (ifs.fail())
			return false;

		// get length of file:
		ifs.seekg(0, std::ios::end);
		int length = ifs.tellg();
		ifs.seekg(0, std::ios::beg);

		// allocate memory:
		char* buffer = new char[length + 1];

		// read data as a block:
		ifs.read(buffer, length);
		ifs.close();

		buffer[length] = 0;

		rapidxml::xml_document<> doc;
		doc.parse<0>(buffer);

		bool is_valid = false;

		rapidxml::xml_node<>* node = doc.first_node("plist");
		if (node)
		{
			node = node->first_node("dict");
			if (node)
			{
				node = node->first_node("dict");
				if (node)
				{
					node = node->next_sibling("dict");
					if (node)
					{
						is_valid = true;
					}
				}
			}
		}

		if (!is_valid)
		{
			delete [] buffer;
			return false;
		}

		// now node is <dict> after <key>frames</key>

		out_map.clear();

		rapidxml::xml_node<>* unit_node;

		TextureAtlasUnit* unit;
		std::string key;

		node = node->first_node();
		while (node)
		{
			if (strcmp(node->name(), "key") == 0)
			{
				key = node->value();

				out_map.insert(std::make_pair(key, TextureAtlasUnit()));

				unit = &out_map[key];
			}
			else if (strcmp(node->name(), "dict") == 0)
			{
				unit_node = node->first_node();
				if (!unit_node || strcmp(unit_node->value(), "x") != 0)
				{
					is_valid = false;
					break;
				}
				unit_node = unit_node->next_sibling();
				if (!unit_node)
				{
					is_valid = false;
					break;
				}

				unit->x = atoi(unit_node->value());

				unit_node = unit_node->next_sibling();
				if (!unit_node || strcmp(unit_node->value(), "y") != 0)
				{
					is_valid = false;
					break;
				}
				unit_node = unit_node->next_sibling();
				if (!unit_node)
				{
					is_valid = false;
					break;
				}

				unit->y = atoi(unit_node->value());

				unit_node = unit_node->next_sibling();
				if (!unit_node || strcmp(unit_node->value(), "width") != 0)
				{
					is_valid = false;
					break;
				}
				unit_node = unit_node->next_sibling();
				if (!unit_node)
				{
					is_valid = false;
					break;
				}

				unit->width = atoi(unit_node->value());

				unit_node = unit_node->next_sibling();
				if (!unit_node || strcmp(unit_node->value(), "height") != 0)
				{
					is_valid = false;
					break;
				}
				unit_node = unit_node->next_sibling();
				if (!unit_node)
				{
					is_valid = false;
					break;
				}

				unit->height = atoi(unit_node->value());
			}

			node = node->next_sibling();
		}

		delete [] buffer;

		if (!is_valid || out_map.empty())
			return false;

		return true;
	}

	int GetUnicodeFromUTF8(const std::string& str, int max_buff_length, uint32_t* buff)
	{
		// TODO: implement ...

		return 0;
	}

	const char* GetLocale()
	{
		static std::string locale;

		JavaCaller caller;
		caller.Set(g_class_name, "GetLocale", "()Ljava/lang/String;");
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
