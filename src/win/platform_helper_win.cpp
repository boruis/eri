/*
 *  platform_helper.cpp
 *  eri
 *
 *  Created by exe on 10/14/10.
 *  Copyright 2010 cobbler. All rights reserved.
 *
 */

#include "platform_helper.h"

#include <algorithm>
#include <fstream>
#include "rapidxml.hpp"

namespace ERI
{
	const char* GetResourcePath()
	{
		static char* path = ".";
		return path;
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

}
