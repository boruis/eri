/*
 *  texture_atlas_mgr.cpp
 *  eri
 *
 *  Created by exe on 2010/10/14.
 *  Copyright 2010 cobbler. All rights reserved.
 *
 */

#include "texture_atlas_mgr.h"

#include <fstream>
#include <algorithm>

#include "scene_actor.h"
#include "platform_helper.h"

#include "rapidxml.hpp"

namespace ERI
{
  
  TextureAtlasMgr* TextureAtlasMgr::ins_ptr_ = NULL;
  
  TextureAtlasMgr::TextureAtlasMgr()
  {
  }
  
  TextureAtlasMgr::~TextureAtlasMgr()
  {
    AtlasArrayMap::iterator array_it = arrays_.begin();
    for (; array_it != arrays_.end(); ++array_it)
      delete array_it->second;

    AtlasMapMap::iterator map_it = maps_.begin();
    for (; map_it != maps_.end(); ++map_it)
      delete map_it->second;
  }
  
  const TextureAtlasArray* TextureAtlasMgr::GetArray(const std::string& name,
                                                     const std::string& prefix /*= ""*/)
  {
    std::string array_name = name;
    
    int prefix_length = static_cast<int>(prefix.length());
    if (prefix_length > 0)
      array_name += "--" + prefix;
    
    AtlasArrayMap::iterator it = arrays_.find(array_name);
    if (it != arrays_.end())
    {
      return it->second;
    }
    else
    {
      if (prefix_length > 0)
      {
        if (NULL == GetMap(name))
          return NULL;
      }
      
      AtlasMapMap::iterator maps_it = maps_.find(name);
      if (maps_it != maps_.end())
      {
        // try get data from loaded map
        
        TextureAtlasMap* map_ref = maps_it->second;

        std::vector<std::string> keys;

        TextureAtlasMap::iterator unit_it = map_ref->begin();
        for (; unit_it != map_ref->end(); ++unit_it)
        {
          if (prefix_length <= 0 ||
              unit_it->first.substr(0, prefix_length).compare(prefix) == 0)
          {
            keys.push_back(unit_it->first);
          }
        }
        
        if (keys.empty())
          return NULL;
        
        std::sort(keys.begin(), keys.end());
        
        TextureAtlasArray* array = new TextureAtlasArray;
        
        for (int i = 0; i < keys.size(); ++i)
        {
          array->push_back((*map_ref)[keys[i]]);
        }
        
        arrays_.insert(std::make_pair(array_name, array));
        
        return array;
      }
      else
      {
        // load from file
        
        TextureAtlasArray* array = new TextureAtlasArray;
        
        if (GetTextureAtlasArray(name, *array))
        {
          arrays_.insert(std::make_pair(name, array));
        }
        else
        {
          delete array;
          array = NULL;
        }
        
        return array;
      }
    }
    
    return NULL;
  }
  
  const TextureAtlasMap* TextureAtlasMgr::GetMap(const std::string& name)
  {
    AtlasMapMap::iterator it = maps_.find(name);
    if (it != maps_.end())
    {
      return it->second;
    }
    else
    {
      // load from file
      
      TextureAtlasMap* map = new TextureAtlasMap;
      
      if (GetTextureAtlasMap(name, *map))
      {
        maps_.insert(std::make_pair(name, map));
      }
      else
      {
        delete map;
        map = NULL;
      }
      
      return map;
    }
    
    return NULL;
  }
  
  bool TextureAtlasMgr::GetTextureAtlasArray(const std::string& name, TextureAtlasArray& out_array)
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
      
      size_t unit_num = for_sort_array.size();
      for (int i = 0; i < unit_num; ++i)
      {
        out_array.push_back(tmp_map[for_sort_array[i]]);
      }
      
      return true;
    }
    
    return false;
  }
  
  static void GetValue(const char* values_str, int& out_value) { out_value = atoi(values_str); }
  static void GetValue(const char* values_str, float& out_value) { out_value = atof(values_str); }
  
  template<typename T>
  static bool GetUnitAttr(const char* name, T& out_value, rapidxml::xml_node<>*& current_node)
  {
    if (!current_node || strcmp(current_node->value(), name) != 0)
      return false;
    
    current_node = current_node->next_sibling();
    if (!current_node)
      return false;
    
    GetValue(current_node->value(), out_value);
    
    current_node = current_node->next_sibling();
    
    return true;
  }
  
  bool TextureAtlasMgr::GetTextureAtlasMap(const std::string& name, TextureAtlasMap& out_map)
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
    long long length = ifs.tellg();
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
    
    TextureAtlasUnit* unit = NULL;
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
        ASSERT(unit);
        
        unit_node = node->first_node();
        
        is_valid = GetUnitAttr("x", unit->x, unit_node);
        if (!is_valid) break;
        is_valid = GetUnitAttr("y", unit->y, unit_node);
        if (!is_valid) break;
        is_valid = GetUnitAttr("width", unit->width, unit_node);
        if (!is_valid) break;
        is_valid = GetUnitAttr("height", unit->height, unit_node);
        if (!is_valid) break;
        is_valid = GetUnitAttr("offsetX", unit->offset_x, unit_node);
        if (!is_valid) break;
        is_valid = GetUnitAttr("offsetY", unit->offset_y, unit_node);
        if (!is_valid) break;
      }
      
      node = node->next_sibling();
    }
    
    delete [] buffer;
    
    if (!is_valid || out_map.empty())
      return false;
    
    return true;
  }
  
  bool ApplyTextureAtlas(const TextureAtlasMap* atlas_map, const std::string& name, SpriteActor* sprite)
  {
    ASSERT(atlas_map && !name.empty() && sprite);
    
    TextureAtlasMap::const_iterator it = atlas_map->find(name);
    if (it != atlas_map->end())
    {
      sprite->SetTexArea(it->second.x, it->second.y, it->second.width, it->second.height);
      return true;
    }
    
    return false;
  }

}
