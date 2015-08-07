/*
 *  texture_atlas_mgr.h
 *  eri
 *
 *  Created by exe on 2010/10/14.
 *  Copyright 2010 cobbler. All rights reserved.
 *
 */

#ifndef ERI_TEXTURE_ATLAS_MGR_H
#define ERI_TEXTURE_ATLAS_MGR_H

#include <string>
#include <vector>
#include <map>

namespace ERI
{
  class SpriteActor;
  
  struct TextureAtlasUnit
  {
    int x, y, width, height;
    float offset_x, offset_y;
  };
  
  typedef std::vector<TextureAtlasUnit> TextureAtlasArray;
  typedef std::map<std::string, TextureAtlasUnit> TextureAtlasMap;

  class TextureAtlasMgr
  {
  public:
    ~TextureAtlasMgr();
    
    const TextureAtlasArray* GetArray(const std::string& name, const std::string& prefix = "");
    const TextureAtlasMap* GetMap(const std::string& name);
    
    inline static TextureAtlasMgr& Ins()
    {
      if (!ins_ptr_) ins_ptr_ = new TextureAtlasMgr;
      return *ins_ptr_;
    }
    
    inline static void DestroyIns()
    {
      if (ins_ptr_)
      {
        delete ins_ptr_;
        ins_ptr_ = NULL;
      }
    }
    
  private:
    TextureAtlasMgr();
    
    bool GetTextureAtlasArray(const std::string& name, TextureAtlasArray& out_array);
    bool GetTextureAtlasMap(const std::string& name, TextureAtlasMap& out_map);
    
    typedef std::map<std::string, TextureAtlasArray*> AtlasArrayMap;
    typedef std::map<std::string, TextureAtlasMap*> AtlasMapMap;
    
    AtlasArrayMap arrays_;
    AtlasMapMap maps_;
    
    static TextureAtlasMgr* ins_ptr_;
  };
  
  bool ApplyTextureAtlas(const TextureAtlasMap* atlas_map, const std::string& name, SpriteActor* sprite);

}

#endif // ERI_TEXTURE_ATLAS_MGR_H
