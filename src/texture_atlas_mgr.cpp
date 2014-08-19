/*
 *  texture_atlas_mgr.cpp
 *  eri
 *
 *  Created by exe on 2010/10/14.
 *  Copyright 2010 cobbler. All rights reserved.
 *
 */

#include "texture_atlas_mgr.h"

#include <algorithm>

#include "scene_actor.h"
#include "platform_helper.h"

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
