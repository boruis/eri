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
#include "font_mgr.h"

namespace ERI
{

class TxtMeshConstructor;
  
class TxtActor : public SceneActor
{
 public:
  TxtActor(const std::string& font_path, int font_size,
           bool is_pos_center = false);
  
  virtual ~TxtActor();
  
  void SetIsUtf8(bool is_utf8);
  void SetIsAntiAlias(bool is_anti_alias);
  void SetResolutionScale(float tex_size_scale);
  
  void SetMaxWidth(float max_width, LineBreakMode line_break = LB_DEFAULT);
  
  void SetTxt(const std::string& txt);
  void SetForceLineHeight(float force_line_height, bool construct = false);
  
  inline const std::string& txt() { return data_.str; }

  inline float width() { return width_; }
  inline float height() { return height_; }

  inline void set_area_border(float border) { area_border_.x = area_border_.y = border; }
  inline void set_area_border(float border_x, float border_y) { area_border_.x = border_x; area_border_.y = border_y; }

 private:
  virtual bool IsInArea(const Vector3& local_space_pos);
  
  const Font* font_ref_;
  
  int font_size_;
  
  TxtData data_;
  
  float resolution_scale_;
  float max_width_;
  
  TxtMeshConstructor* mesh_constructor_;
  
  float width_, height_;
  Vector2 area_border_;
  
  float force_line_height_;

  friend class SpriteTxtMeshConstructor;
  friend class AtlasTxtMeshConstructor;
};

}

#endif // ERI_TXT_ACTOR_H
