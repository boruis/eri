//
//  txt_actor.h
//  eri
//
//  Created by exe on 4/24/11.
//  Copyright 2011 cobbler. All rights reserved.
//

#ifndef ERI_TXT_ACTOR_H
#define ERI_TXT_ACTOR_H

#include "scene_actor.h"

namespace ERI
{

class Font;
class TxtMeshConstructor;
  
class TxtActor : public SceneActor
{
 public:
  TxtActor(const std::string& font_path, int font_size,
           bool is_pos_center = false,
           bool is_utf8 = false,
           bool is_anti_alias = true);
  
  virtual ~TxtActor();
  
  void SetTxt(const std::string& txt);
  
  inline const std::string& txt() { return txt_; }

  inline float width() { return width_; }
  inline float height() { return height_; }
    
  inline void set_area_border(float size) { area_border_ = size; }
  
  static void CalculateSize(const std::string& txt,
                            const Font* font,
                            int font_size,
                            float& width,
                            float& height,
                            bool is_utf8 = false);
  
  static void CalculateSize(const uint32_t* chars,
                            int length,
                            const Font* font,
                            int font_size,
                            float& width,
                            float& height);

 private:
  virtual bool IsInArea(const Vector3& local_space_pos);
  
  const Font* font_ref_;
  
  int   font_size_;
  bool  is_pos_center_;  
  bool	is_utf8_;
  bool  is_anti_alias_;

  std::string txt_;
  
  TxtMeshConstructor* mesh_constructor_;
  
  float width_, height_;
  float area_border_;

  friend class SpriteTxtMeshConstructor;
  friend class AtlasTxtMeshConstructor;
};

}

#endif // ERI_TXT_ACTOR_H
