//
//  txt_actor.cpp
//  eri
//
//  Created by exe on 4/24/11.
//  Copyright 2011 cobbler. All rights reserved.
//

#include "pch.h"

#include "txt_actor.h"

#include "root.h"
#include "font_mgr.h"

#include "platform_helper.h"

namespace ERI
{

#pragma mark TxtMeshConstructor
  
class TxtMeshConstructor
{
 public:
  TxtMeshConstructor(TxtActor* owner)
    : owner_(owner),
      vertices_(NULL)
  {
    ASSERT(owner_);
  }
  
  virtual ~TxtMeshConstructor()
  {
    if (vertices_) free(vertices_);
  }

  virtual void Construct() = 0;
  
 protected:
  TxtActor* owner_;
  vertex_2_pos_tex* vertices_;
};
  
#pragma mark SpriteTxtMeshConstructor

class SpriteTxtMeshConstructor : public TxtMeshConstructor
{
 public:
  SpriteTxtMeshConstructor(TxtActor* owner)
    : TxtMeshConstructor(owner)
  {
    char tex_name[32];
    sprintf(tex_name, "txt:%p", owner_);
    tex_name_ = tex_name;
  }
  
  virtual ~SpriteTxtMeshConstructor()
  {
    owner_->SetMaterial(NULL);
    Root::Ins().texture_mgr()->ReleaseTexture(tex_name_);
  }
  
  virtual void Construct()
  {
    Root::Ins().renderer()->SetContextAsCurrent();

    owner_->SetMaterial(NULL);
    
    const Font* font = owner_->font_ref_;
    const TxtData& data = owner_->data_;
    
    int font_size = Round(owner_->font_size_ * owner_->resolution_scale_);
    float resolution_scale = static_cast<float>(font_size) / owner_->font_size_;
    int max_width = Round(owner_->max_width_ * owner_->resolution_scale_);
    
    int width, height;
    const Texture* tex = font->CreateSpriteTxt(tex_name_,
                                               data,
                                               font_size,
                                               max_width,
                                               width,
                                               height);

    owner_->SetMaterial(tex,
                        owner_->font_ref_->filter_min(),
                        owner_->font_ref_->filter_mag());
    
    owner_->SetTextureWrap(owner_->font_ref_->wrap_s(),
                           owner_->font_ref_->wrap_t());
    
    owner_->width_ = Round(width / resolution_scale);
    owner_->height_ = Round(height / resolution_scale);
    
    if (owner_->render_data_.vertex_buffer == 0)
		{
			glGenBuffers(1, &owner_->render_data_.vertex_buffer);
		}
		    
    float size_scale = font->GetSizeScale(owner_->font_size_);

    Vector2 size(Round(owner_->width_ * size_scale),
                 Round(owner_->height_ * size_scale));
    
    Vector2 start;
    if (data.is_pos_center)
    {
      start.x =  Round(-size.x * 0.5f);
      start.y =  Round(size.y * 0.5f);
    }
    
    Vector2 uv_size(static_cast<float>(width) / tex->width,
                    static_cast<float>(height) / tex->height);
    
    //  2 - 3
    //  | \ |
    //  0 - 1
    
    vertex_2_pos_tex v[] = {
      { start.x,          start.y - size.y, 0.0f,       uv_size.y },
      { start.x + size.x, start.y - size.y, uv_size.x,  uv_size.y },
      { start.x,          start.y         , 0.0f,       0.0f      },
      { start.x + size.x, start.y         , uv_size.x,  0.0f      }
    };

    owner_->render_data_.vertex_type = GL_TRIANGLE_STRIP;
    owner_->render_data_.vertex_format = POS_TEX_2;
    owner_->render_data_.vertex_count = 4;
    
    glBindBuffer(GL_ARRAY_BUFFER, owner_->render_data_.vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
  }
  
 private:
  std::string tex_name_;
};
  
#pragma mark AtlasTxtMeshConstructor
  
class AtlasTxtMeshConstructor : public TxtMeshConstructor
{
 public:
  AtlasTxtMeshConstructor(TxtActor* owner)
    : TxtMeshConstructor(owner),
      now_len_max_(0),
      now_len_(0)
  {
  }
  
  virtual ~AtlasTxtMeshConstructor()
  {
  }
  
  virtual void Construct()
  {
    Root::Ins().renderer()->SetContextAsCurrent();
    
    const Font* font = owner_->font_ref_;
    const TxtData& data = owner_->data_;

    uint32_t* chars;
    now_len_ = CreateUnicodeArray(data, chars);
    
    int unit_vertex_num = 6;
    
    if (now_len_max_ < now_len_)
    {
      now_len_max_ = now_len_;
      
      if (vertices_) free(vertices_);
      vertices_ = static_cast<vertex_2_pos_tex*>(malloc(now_len_max_ * unit_vertex_num * sizeof(vertex_2_pos_tex)));
      
      if (owner_->render_data_.vertex_buffer != 0)
      {
        glDeleteBuffers(1, &owner_->render_data_.vertex_buffer);
      }
      
      glGenBuffers(1, &owner_->render_data_.vertex_buffer);
    }
    
    float inv_tex_width = 1.0f / font->texture()->width;
    float inv_tex_height = 1.0f / font->texture()->height;
    float size_scale = font->GetSizeScale(owner_->font_size_);
    
    std::vector<float> row_widths;
    
    CalculateTxtSize(chars,
                     now_len_,
                     owner_->font_ref_,
                     owner_->font_size_,
                     owner_->max_width_,
                     owner_->width_,
                     owner_->height_,
                     &row_widths);
    
    float line_height = font->common_line_height() * size_scale;
    if (owner_->force_line_height_ > 0.f)
    {
      owner_->height_ = (row_widths.size() - 1) * owner_->force_line_height_ + line_height;
      line_height = owner_->force_line_height_;
    }
    
    int row = 0;
    float start_x = data.is_pos_center ? row_widths[row] * -0.5f : 0;
    float start_y = data.is_pos_center ? owner_->height_ * 0.5f : 0;
    int start_idx = 0;
    float scroll_u, scroll_v, unit_u, unit_v, offset_x, offset_y, size_x, size_y;
    
    int invisible_num = 0;
    
    vertex_2_pos_tex* v;
    
    for (int i = 0; i < now_len_; ++i)
    {
      if (chars[i] == '\n')
      {
        ++row;
        start_x = data.is_pos_center ? row_widths[row] * -0.5f : 0;
        start_y -= line_height;
        ++invisible_num;
      }
      else
      {
        const CharSetting& setting = font->GetCharSetting(chars[i]);
        
        if (owner_->max_width_ > 0.f &&
            (start_x + setting.x_advance * size_scale) > owner_->max_width_)
        {
          ++row;
          start_x = data.is_pos_center ? row_widths[row] * -0.5f : 0;
          start_y -= line_height;
        }
        
        scroll_u = setting.x * inv_tex_width;
        scroll_v = setting.y * inv_tex_height;
        unit_u = (setting.width - 1) * inv_tex_width;
        unit_v = (setting.height - 1) * inv_tex_height;
        
        offset_x = setting.x_offset * size_scale;
        offset_y = setting.y_offset * size_scale;
        size_x = (setting.width - 1) * size_scale;
        size_y = (setting.height - 1) * size_scale;
        
        v = &vertices_[start_idx];
        
        // 2,3 -  5
        //  |  \  |
        //  0 -  1,4
        
        v[0].position[0] = start_x + offset_x;
        v[0].position[1] = start_y - offset_y - size_y;
        v[0].tex_coord[0] = scroll_u;
        v[0].tex_coord[1] = scroll_v + unit_v;
        
        v[1].position[0] = start_x + offset_x + size_x;
        v[1].position[1] = start_y - offset_y - size_y;
        v[1].tex_coord[0] = scroll_u + unit_u;
        v[1].tex_coord[1] = scroll_v + unit_v;
        
        v[2].position[0] = start_x + offset_x;
        v[2].position[1] = start_y - offset_y;
        v[2].tex_coord[0] = scroll_u;
        v[2].tex_coord[1] = scroll_v;
        
        v[3].position[0] = start_x + offset_x;
        v[3].position[1] = start_y - offset_y;
        v[3].tex_coord[0] = scroll_u;
        v[3].tex_coord[1] = scroll_v;
        
        v[4].position[0] = start_x + offset_x + size_x;
        v[4].position[1] = start_y - offset_y - size_y;
        v[4].tex_coord[0] = scroll_u + unit_u;
        v[4].tex_coord[1] = scroll_v + unit_v;
        
        v[5].position[0] = start_x + offset_x + size_x;
        v[5].position[1] = start_y - offset_y;
        v[5].tex_coord[0] = scroll_u + unit_u;
        v[5].tex_coord[1] = scroll_v;
        
        start_x += setting.x_advance * size_scale;
        start_idx += unit_vertex_num;
      }
    }
    
    delete [] chars;
    
    owner_->render_data_.vertex_count = (now_len_ - invisible_num) * unit_vertex_num;
    owner_->render_data_.vertex_type = GL_TRIANGLES;
    
    glBindBuffer(GL_ARRAY_BUFFER, owner_->render_data_.vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER,
                 owner_->render_data_.vertex_count * sizeof(vertex_2_pos_tex),
                 vertices_,
                 GL_DYNAMIC_DRAW);
  }
  
 private:
  int now_len_max_;
  int now_len_;
};

#pragma mark TxtActor
  
TxtActor::TxtActor(const std::string& font_path, int font_size,
                   bool is_pos_center /*= false*/)
  : font_ref_(NULL),
    font_size_(font_size),
    resolution_scale_(1.f),
    max_width_(0.f),
    width_(0.0f),
    height_(0.0f),
    force_line_height_(0.0f)
{
  data_.is_pos_center = is_pos_center;
  
  font_ref_ = Root::Ins().font_mgr()->GetFont(font_path);
  
  ASSERT(font_ref_);

  SetIsUtf8(font_ref_->is_utf8()); // TODO: bad rule
  
  if (font_ref_->is_atlas())
  {
    mesh_constructor_ = new AtlasTxtMeshConstructor(this);
    
    SetMaterial(font_ref_->texture(),
                font_ref_->filter_min(),
                font_ref_->filter_mag());
  }
  else
  {
    mesh_constructor_ = new SpriteTxtMeshConstructor(this);
  }
}

TxtActor::~TxtActor()
{
  delete mesh_constructor_;
}
  
void TxtActor::SetIsUtf8(bool is_utf8)
{
  if (data_.is_utf8 == is_utf8)
    return;
  
  data_.is_utf8 = is_utf8;
  
  if (!data_.str.empty())
    mesh_constructor_->Construct();
}

void TxtActor::SetIsAntiAlias(bool is_anti_alias)
{
  if (data_.is_anti_alias == is_anti_alias)
    return;
  
  data_.is_anti_alias = is_anti_alias;
  
  if (!data_.str.empty())
    mesh_constructor_->Construct();
}

void TxtActor::SetResolutionScale(float resolution_scale)
{
  if (resolution_scale_ == resolution_scale)
    return;
  
  resolution_scale_ = resolution_scale;
  
  if (!data_.str.empty())
    mesh_constructor_->Construct();
}

void TxtActor::SetMaxWidth(float max_width, LineBreakMode line_break /*= LB_DEFAULT*/)
{
  ASSERT(max_width > 0.f);

  if (max_width_ == max_width && data_.line_break == line_break)
    return;
  
  max_width_ = max_width;
  data_.line_break = line_break;
  
  if (!data_.str.empty())
    mesh_constructor_->Construct();
}

void TxtActor::SetTxt(const std::string& txt)
{
  data_.str = txt;
  
  mesh_constructor_->Construct();
}
  
void TxtActor::SetForceLineHeight(float force_line_height, bool construct /*= false*/)
{
  ASSERT(force_line_height >= 0.f);
  
  if (force_line_height == force_line_height_)
    return;
  
  force_line_height_ = force_line_height;
  
  if (construct)
    mesh_constructor_->Construct();
}

bool TxtActor::IsInArea(const Vector3& local_space_pos)
{
  if (local_space_pos.x >= ((data_.is_pos_center ? (-width_ / 2) : 0) - area_border_.x)
			&& local_space_pos.x <= ((data_.is_pos_center ? (width_ / 2) : width_) + area_border_.x)
			&& local_space_pos.y >= ((data_.is_pos_center ? (-height_ / 2) : -height_) - area_border_.y)
			&& local_space_pos.y <= ((data_.is_pos_center ? (height_ / 2) : 0) + area_border_.y))
  {
    return true;
  }
  
  return false;
}
  
}
