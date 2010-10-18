/*
 *  scene_actor.cpp
 *  eri
 *
 *  Created by exe on 11/29/09.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#include "pch.h"

#include "scene_actor.h"
#include "scene_mgr.h"
#include "render_data.h"
#include "renderer.h"
#include "texture_mgr.h"
#include "font_mgr.h"
#include "root.h"

namespace ERI {
	
#pragma mark SceneActor
	
	SceneActor::SceneActor() :
		layer_id_(-1),
		parent_(NULL),
		visible_(true),
		user_data_(NULL)
	{
	}
	
	SceneActor::~SceneActor()
	{
		for (size_t i = 0; i < childs_.size(); ++i)
		{
			// TODO: is delete childs right?

			childs_[i]->parent_ = NULL;
			delete childs_[i];
		}
		
		if (parent_)
		{
			parent_->RemoveChild(this);
		}
		
		if (layer_id_ != -1)
		{
			Root::Ins().scene_mgr()->RemoveActor(this, layer_id_);
		}
	}
	
	void SceneActor::AddToScene(int layer_id /*= 0*/)
	{
		ASSERT(!parent_);
		ASSERT(layer_id_ == -1);
			
		Root::Ins().scene_mgr()->AddActor(this, layer_id);
		layer_id_ = layer_id;
	}
	
	void SceneActor::RemoveFromScene()
	{
		ASSERT(!parent_);
		ASSERT(layer_id_ != -1);
		
		Root::Ins().scene_mgr()->RemoveActor(this, layer_id_);
		layer_id_ = -1;
	}
	
	void SceneActor::MoveToLayer(int layer_id)
	{
		RemoveFromScene();
		AddToScene(layer_id);
	}
	
	void SceneActor::AddChild(SceneActor* actor)
	{
		ASSERT(actor);
		
		if (actor->parent_)
			actor->parent_->RemoveChild(actor);
		
		childs_.push_back(actor);
		actor->parent_ = this;
	}
	
	void SceneActor::RemoveChild(SceneActor* actor)
	{
		ASSERT(actor);
		
		size_t num = childs_.size();
		size_t i;
		for (i = 0; i < num; ++i)
		{
			if (childs_[i] == actor)
			{
				if (i < num - 1)
					childs_[i] = childs_[num - 1];
				
				childs_.pop_back();
				break;
			}
		}
		
		ASSERT(i < num);
		
		actor->parent_ = NULL;
	}
	
	bool SceneActor::IsHit(const Vector3& world_space_pos)
	{
		std::vector<SceneActor*> parent_list;
		SceneActor* now_parent = parent_;
		while (now_parent)
		{
			parent_list.push_back(now_parent);
			now_parent = now_parent->parent_;
		}

		Vector3 local_space_pos = world_space_pos;
		
		for (int i = parent_list.size() - 1; i >= 0; --i)
		{
			local_space_pos = parent_list[i]->GetInvTransform() * local_space_pos;
		}
		
		if (render_data_.need_update_model_matrix)
			render_data_.UpdateModelMatrix();
		
		local_space_pos = render_data_.inv_model_matrix * local_space_pos;
		
		return IsInArea(local_space_pos);
	}
	
	SceneActor* SceneActor::GetHitActor(const Vector3& parent_space_pos)
	{
		if (!visible_)
			return NULL;
		
		if (render_data_.need_update_model_matrix)
			render_data_.UpdateModelMatrix();

		Vector3 local_space_pos = render_data_.inv_model_matrix * parent_space_pos;
		
		if (IsInArea(local_space_pos))
			return this;

		if (!childs_.empty())
		{
			SceneActor* actor;
			for (int i = childs_.size() - 1; i >= 0; --i)
			{
				actor = childs_[i]->GetHitActor(local_space_pos);
				if (actor)
					return actor;
			}
		}
		
		return NULL;
	}
	
	void SceneActor::Render(Renderer* renderer)
	{
		if (!visible_)
			return;
		
		// set material state
		
		renderer->EnableMaterial(&material_data_);
		
		renderer->SaveTransform();

		GetWorldTransform();
		
		renderer->Render(&render_data_);
	
		renderer->RecoverTransform();
	}
	
	void SceneActor::SetColor(const Color& color)
	{
		render_data_.color = color;
	}
	
	const Color& SceneActor::GetColor()
	{
		return render_data_.color;
	}
	
	void SceneActor::BlendAdd()
	{
		render_data_.blend_src_factor = GL_SRC_ALPHA;
		render_data_.blend_dst_factor = GL_ONE;
	}
	
	const Matrix4& SceneActor::GetTransform()
	{
		if (render_data_.need_update_model_matrix)
			render_data_.UpdateModelMatrix();

		return render_data_.model_matrix;
	}

	const Matrix4& SceneActor::GetInvTransform()
	{
		if (render_data_.need_update_model_matrix)
			render_data_.UpdateModelMatrix();

		return render_data_.inv_model_matrix;
	}
	
	const Matrix4& SceneActor::GetWorldTransform()
	{
		if (render_data_.need_update_model_matrix)
			render_data_.UpdateModelMatrix();
		
		if (render_data_.need_update_world_model_matrix)
		{
			if (parent_)
			{
				render_data_.UpdateWorldModelMatrix(parent_->GetWorldTransform());
			}
			else render_data_.UpdateWorldModelMatrix();
		}
		
		return render_data_.world_model_matrix;
	}
	
	void SceneActor::SetPos(float x, float y)
	{
		render_data_.translate.x = x;
		render_data_.translate.y = y;
		SetTransformDirty();
	}
	
	Vector2 SceneActor::GetPos()
	{
		return Vector2(render_data_.translate.x, render_data_.translate.y);
	}
	
	void SceneActor::SetRotate(float degree)
	{
		render_data_.rotate_degree = degree;
		render_data_.rotate_axis = Vector3(0, 0, 1);
		SetTransformDirty();
	}
	
	float SceneActor::GetRotate() const
	{
		return render_data_.rotate_degree;
	}
	
	void SceneActor::SetScale(float x, float y)
	{
		render_data_.scale.x = x;
		render_data_.scale.y = y;
		SetTransformDirty();
	}
	
	Vector2 SceneActor::GetScale()
	{
		return Vector2(render_data_.scale.x, render_data_.scale.y);
	}
	
	void SceneActor::SetPos(const Vector3& pos)
	{
		render_data_.translate = pos;
		render_data_.need_update_model_matrix = true;
		render_data_.need_update_world_model_matrix = true;
	}
	
	const Vector3& SceneActor::GetPos3()
	{
		return render_data_.translate;
	}
	
	void SceneActor::SetRotate(float degree, const Vector3& axis)
	{
		render_data_.rotate_degree = degree;
		render_data_.rotate_axis = axis;
		SetTransformDirty();
	}
	
	void SceneActor::SetScale(const Vector3& scale)
	{
		render_data_.scale = scale;
		SetTransformDirty();
	}
	
	void SceneActor::SetMaterial(const std::string& texture_path, TextureFilter filter_min /*= FILTER_NEAREST*/, TextureFilter filter_mag /*= FILTER_NEAREST*/)
	{
		const Texture* tex = NULL;

		if (texture_path.length() > 0)
		{
			tex = Root::Ins().texture_mgr()->GetTexture(texture_path);
		}

		SetTexture(0, tex);

		if (tex)
		{
			if (material_data_.used_unit < 1)
				material_data_.used_unit = 1;

			material_data_.texture_units[0].params.filter_min = filter_min;
			material_data_.texture_units[0].params.filter_mag = filter_mag;
		}
		else if (material_data_.used_unit == 1)
		{
			material_data_.used_unit = 0;
		}
	}
	
	void SceneActor::SetMaterial(const Texture* tex, TextureFilter filter_min /*= FILTER_NEAREST*/, TextureFilter filter_mag /*= FILTER_NEAREST*/)
	{
		SetTexture(0, tex);

		if (tex)
		{
			if (material_data_.used_unit < 1)
				material_data_.used_unit = 1;

			material_data_.texture_units[0].params.filter_min = filter_min;
			material_data_.texture_units[0].params.filter_mag = filter_mag;
		}
		else if (material_data_.used_unit == 1)
		{
			material_data_.used_unit = 0;
		}
	}
	
	void SceneActor::AddMaterial(const std::string& texture_path, TextureFilter filter_min /*= FILTER_NEAREST*/, TextureFilter filter_mag /*= FILTER_NEAREST*/)
	{
		ASSERT(material_data_.used_unit < MAX_TEXTURE_UNIT);
		
		int idx = material_data_.used_unit;
		SetTexture(idx, Root::Ins().texture_mgr()->GetTexture(texture_path));
		
		++material_data_.used_unit;
		
		material_data_.texture_units[idx].params.filter_min = filter_min;
		material_data_.texture_units[idx].params.filter_mag = filter_mag;
	}
	
	void SceneActor::AddMaterial()
	{
		ASSERT(material_data_.used_unit < MAX_TEXTURE_UNIT);
		
		++material_data_.used_unit;
	}
	
	void SceneActor::SetTextureFilter(TextureFilter filter_min, TextureFilter filter_mag)
	{
		ASSERT(material_data_.used_unit > 0);
		
		material_data_.texture_units[0].params.filter_min = filter_min;
		material_data_.texture_units[0].params.filter_mag = filter_mag;
	}
	
	void SceneActor::SetTextureEnvs(int idx, const TextureEnvs& envs)
	{
		ASSERT(idx >= 0 && idx < material_data_.used_unit);
		
		material_data_.texture_units[idx].envs = envs;
	}
	
	void SceneActor::SetOpacityType(OpacityType type)
	{
		if (material_data_.opacity_type != type)
		{
			if (layer_id_ != -1)
			{
				Root::Ins().scene_mgr()->RemoveActor(this, layer_id_);
				material_data_.opacity_type = type;
				Root::Ins().scene_mgr()->AddActor(this, layer_id_);
			}
			else
			{
				material_data_.opacity_type = type;
			}
		}
	}
	
	void SceneActor::SetDepthTest(bool enable)
	{
		material_data_.depth_test = enable;
	}
	
	void SceneActor::SetDepthWrite(bool enable)
	{
		material_data_.depth_write = enable;
	}
	
	void SceneActor::SetTransformDirty()
	{
		render_data_.need_update_model_matrix = true;

		SetWorldTransformDirty();
	}
	
	void SceneActor::SetWorldTransformDirty()
	{
		render_data_.need_update_world_model_matrix = true;

		int child_num = childs_.size();
		for (int i = 0; i < child_num; ++i)
		{
			childs_[i]->SetWorldTransformDirty();
		}
	}
	
	void SceneActor::SetTexture(int idx, const Texture* tex)
	{
		if (material_data_.texture_units[idx].texture != tex)
		{
			if (idx == 0 && layer_id_ != -1)
			{
				Root::Ins().scene_mgr()->RemoveActor(this, layer_id_);
				material_data_.texture_units[idx].texture = tex;
				Root::Ins().scene_mgr()->AddActor(this, layer_id_);
			}
			else
			{
				material_data_.texture_units[idx].texture = tex;
			}
		}
	}
	
#pragma mark CameraActor
	
	CameraActor::CameraActor(Projection projection) :
		projection_(projection),
		look_at_(Vector3(0, 0, -1)),
		is_look_at_offset_(true),
		ortho_zoom_(1.0f),
		perspective_fov_y_(Math::PI / 3.0f),
		is_view_modified_(true),
		is_projection_modified_(true)
	{
	}
	
	CameraActor::~CameraActor()
	{
	}
	
	void CameraActor::SetPos(float x, float y)
	{
		SceneActor::SetPos(x, y);
		
		is_view_modified_ = true;
	}
	
	void CameraActor::SetPos(const Vector3& pos)
	{
		SceneActor::SetPos(pos);
		
		is_view_modified_ = true;
	}
	
	void CameraActor::SetLookAt(const Vector3& look_at, bool is_offset)
	{
		look_at_ = look_at;
		is_look_at_offset_ = is_offset;
		
		is_view_modified_ = true;
		
		// TODO: modify self rotation to make childs' transform correct
	}
	
	void CameraActor::SetOrthoZoom(float zoom)
	{
		ASSERT(projection_ == ORTHOGONAL);
		ASSERT(zoom > 0);
		
		ortho_zoom_ = zoom;
		SetScale(Vector3(1 / ortho_zoom_, 1 / ortho_zoom_, 1 / ortho_zoom_));
		//SetScale(1 / ortho_zoom_, 1 / ortho_zoom_);
		
		is_projection_modified_ = true;
	}
	
	void CameraActor::SetPerspectiveFov(float fov_y)
	{
		ASSERT(projection_ == PERSPECTIVE);
		ASSERT(fov_y > 0);
		
		perspective_fov_y_ = fov_y;
		
		is_projection_modified_ = true;
	}
	
	void CameraActor::UpdateViewMatrix()
	{
		ASSERT(is_view_modified_);
		
		const Vector3& pos = GetPos3();
		Root::Ins().renderer()->UpdateView(pos, is_look_at_offset_ ? (pos + look_at_) : look_at_, Vector3(0, 1, 0));
		
		is_view_modified_ = false;
	}
	
	void CameraActor::UpdateProjectionMatrix()
	{
		ASSERT(is_projection_modified_);
		
		if (projection_ == ORTHOGONAL)
		{
			Root::Ins().renderer()->UpdateOrthoProjection(ortho_zoom_, -1000, 1000);
		}
		else
		{
			Root::Ins().renderer()->UpdatePerspectiveProjection(perspective_fov_y_, 1, 1000);
		}
		
		is_projection_modified_ = false;
	}
	
	void CameraActor::SetViewProjectionModified()
	{
		is_view_modified_ = true;
		is_projection_modified_ = true;
	}
	
	void CameraActor::SetViewModified()
	{
		is_view_modified_ = true;
	}
	
	void CameraActor::SetProjectionModified()
	{
		is_projection_modified_ = true;
	}

#pragma mark LightActor
	
	LightActor::LightActor(Type type) :
		type_(type),
		idx_(-1),
		ambient_(Color(0, 0, 0)),
		diffuse_(Color(0, 0, 0)),
		specular_(Color(0, 0, 0)),
		attenuation_constant_(1.0f),
		attenuation_linear_(0.0f),
		attenuation_quadratic_(0.0f),
		dir_(Vector3(0, 0, -1)),
		spot_exponent_(0.0f),
		spot_cutoff_(180.0f)
	{
	}
	
	LightActor::~LightActor()
	{
		if (idx_ != -1)
		{
			Root::Ins().renderer()->ReleaseLight(idx_);
			idx_ = -1;
		}
	}
	
	void LightActor::AddToScene(int layer_id)
	{
		ASSERT(idx_ == -1);
		
		Root::Ins().renderer()->ObtainLight(idx_);
		
		ASSERT(idx_ != -1);
		
		if (idx_ != -1)
		{
			SceneActor::AddToScene(layer_id);
			
			// apply settings
			SetPos(GetPos3());
			SetDir(dir_);
			SetAmbient(ambient_);
			SetDiffuse(diffuse_);
			SetSpecular(specular_);
			SetAttenuation(attenuation_constant_, attenuation_linear_, attenuation_quadratic_);
			SetSpotExponent(spot_exponent_);
			SetSpotCutoff(spot_cutoff_);
		}
	}
	
	void LightActor::RemoveFromScene()
	{
		ASSERT(idx_ != -1);
		
		SceneActor::RemoveFromScene();
		
		Root::Ins().renderer()->ReleaseLight(idx_);
		idx_ = -1;
	}
	
	void LightActor::SetPos(float x, float y)
	{
		SceneActor::SetPos(x, y);
		
		if (idx_ == -1) return;
		
		if (type_ != DIRECTION)
		{
			Root::Ins().renderer()->SetLightPos(idx_, GetPos3());
		}
	}
		
	void LightActor::SetPos(const Vector3& pos)
	{
		SceneActor::SetPos(pos);
		
		if (idx_ == -1) return;
		
		if (type_ != DIRECTION)
		{
			Root::Ins().renderer()->SetLightPos(idx_, pos);
		}
	}
	
	void LightActor::SetDir(const Vector3& dir)
	{
		dir_ = dir;
		
		if (idx_ == -1) return;
		
		if (type_ == DIRECTION)
		{
			Root::Ins().renderer()->SetLightDir(idx_, dir_);
		}
		else if (type_ == SPOT)
		{
			Root::Ins().renderer()->SetLightSpotDir(idx_, dir_);
		}
	}
	
	void LightActor::SetAmbient(const Color& ambient)
	{
		ambient_ = ambient;
		
		if (idx_ == -1) return;
		
		Root::Ins().renderer()->SetLightAmbient(idx_, ambient_);
	}
	
	void LightActor::SetDiffuse(const Color& diffuse)
	{
		diffuse_ = diffuse;
		
		if (idx_ == -1) return;

		Root::Ins().renderer()->SetLightDiffuse(idx_, diffuse_);
	}
	
	void LightActor::SetSpecular(const Color& specular)
	{
		specular_ = specular;
		
		if (idx_ == -1) return;
		
		Root::Ins().renderer()->SetLightSpecular(idx_, specular_);
	}
	
	void LightActor::SetAttenuation(float constant, float linear, float quadratic)
	{
		attenuation_constant_ = constant;
		attenuation_linear_ = linear;
		attenuation_quadratic_ = quadratic;
		
		if (idx_ == -1) return;
		
		Root::Ins().renderer()->SetLightAttenuation(idx_, attenuation_constant_, attenuation_linear_, attenuation_quadratic_);
	}
	
	void LightActor::SetSpotExponent(float exponent)
	{
		spot_exponent_ = exponent;
		
		if (idx_ == -1 || type_ != SPOT) return;
		
		Root::Ins().renderer()->SetLightSpotExponent(idx_, spot_exponent_);
	}
	
	void LightActor::SetSpotCutoff(float cutoff)
	{
		spot_cutoff_ = cutoff;
		
		if (idx_ == -1 || type_ != SPOT) return;
		
		Root::Ins().renderer()->SetLightSpotCutoff(idx_, spot_cutoff_);
	}
					
#pragma mark SpriteActor

	SpriteActor::SpriteActor(float width, float height, float offset_width, float offset_height) :
		tex_scale_(Vector2(1.0f, 1.0f)),
		tex_scale2_(Vector2(1.0f, 1.0f)),
		is_dynamic_draw_(false),
		is_use_line_(false),
		area_border_(0.0f)
	{
		SetSizeOffset(width, height, offset_width, offset_height);
	}
	
	SpriteActor::~SpriteActor()
	{
	}
	
	void SpriteActor::UpdateVertexBuffer()
	{
		if (render_data_.vertex_buffer == 0)
		{
			glGenBuffers(1, &render_data_.vertex_buffer);
		}
		
		glBindBuffer(GL_ARRAY_BUFFER, render_data_.vertex_buffer);
		
		if (is_use_line_)
		{
			if (is_use_tex2_)
			{
				vertex_2_pos_tex2 v[] =
				{
					{ offset_.x - 0.5f * size_.x, offset_.y - 0.5f * size_.y, tex_scroll_.x + 0.0f * tex_scale_.x, tex_scroll_.y + 1.0f * tex_scale_.y, tex_scroll2_.x + 0.0f * tex_scale2_.x, tex_scroll2_.y + 1.0f * tex_scale2_.y },
					{ offset_.x + 0.5f * size_.x, offset_.y - 0.5f * size_.y, tex_scroll_.x + 1.0f * tex_scale_.x, tex_scroll_.y + 1.0f * tex_scale_.y, tex_scroll2_.x + 1.0f * tex_scale2_.x, tex_scroll2_.y + 1.0f * tex_scale2_.y },
					{ offset_.x + 0.5f * size_.x, offset_.y + 0.5f * size_.y, tex_scroll_.x + 1.0f * tex_scale_.x, tex_scroll_.y + 0.0f * tex_scale_.y, tex_scroll2_.x + 1.0f * tex_scale2_.x, tex_scroll2_.y + 0.0f * tex_scale2_.y },
					{ offset_.x - 0.5f * size_.x, offset_.y + 0.5f * size_.y, tex_scroll_.x + 0.0f * tex_scale_.x, tex_scroll_.y + 0.0f * tex_scale_.y, tex_scroll2_.x + 0.0f * tex_scale2_.x, tex_scroll2_.y + 0.0f * tex_scale2_.y }
				};
				
				glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, is_dynamic_draw_ ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
			}
			else
			{
				vertex_2_pos_tex v[] =
				{
					{ offset_.x - 0.5f * size_.x, offset_.y - 0.5f * size_.y, tex_scroll_.x + 0.0f * tex_scale_.x, tex_scroll_.y + 1.0f * tex_scale_.y },
					{ offset_.x + 0.5f * size_.x, offset_.y - 0.5f * size_.y, tex_scroll_.x + 1.0f * tex_scale_.x, tex_scroll_.y + 1.0f * tex_scale_.y },
					{ offset_.x + 0.5f * size_.x, offset_.y + 0.5f * size_.y, tex_scroll_.x + 1.0f * tex_scale_.x, tex_scroll_.y + 0.0f * tex_scale_.y },
					{ offset_.x - 0.5f * size_.x, offset_.y + 0.5f * size_.y, tex_scroll_.x + 0.0f * tex_scale_.x, tex_scroll_.y + 0.0f * tex_scale_.y }
				};
				
				glBufferData(GL_ARRAY_BUFFER, sizeof(v), v,  is_dynamic_draw_ ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
			}
			
			render_data_.vertex_type = GL_LINE_LOOP;
		}
		else
		{
			if (is_use_tex2_)
			{
				vertex_2_pos_tex2 v[] = {
					{ offset_.x - 0.5f * size_.x, offset_.y - 0.5f * size_.y, tex_scroll_.x + 0.0f * tex_scale_.x, tex_scroll_.y + 1.0f * tex_scale_.y, tex_scroll2_.x + 0.0f * tex_scale2_.x, tex_scroll2_.y + 1.0f * tex_scale2_.y },
					{ offset_.x + 0.5f * size_.x, offset_.y - 0.5f * size_.y, tex_scroll_.x + 1.0f * tex_scale_.x, tex_scroll_.y + 1.0f * tex_scale_.y, tex_scroll2_.x + 1.0f * tex_scale2_.x, tex_scroll2_.y + 1.0f * tex_scale2_.y },
					{ offset_.x - 0.5f * size_.x, offset_.y + 0.5f * size_.y, tex_scroll_.x + 0.0f * tex_scale_.x, tex_scroll_.y + 0.0f * tex_scale_.y, tex_scroll2_.x + 0.0f * tex_scale2_.x, tex_scroll2_.y + 0.0f * tex_scale2_.y },
					{ offset_.x + 0.5f * size_.x, offset_.y + 0.5f * size_.y, tex_scroll_.x + 1.0f * tex_scale_.x, tex_scroll_.y + 0.0f * tex_scale_.y, tex_scroll2_.x + 1.0f * tex_scale2_.x, tex_scroll2_.y + 0.0f * tex_scale2_.y }
				};
				
				glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, is_dynamic_draw_ ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
			}
			else
			{
				vertex_2_pos_tex v[] = {
					{ offset_.x - 0.5f * size_.x, offset_.y - 0.5f * size_.y, tex_scroll_.x + 0.0f * tex_scale_.x, tex_scroll_.y + 1.0f * tex_scale_.y },
					{ offset_.x + 0.5f * size_.x, offset_.y - 0.5f * size_.y, tex_scroll_.x + 1.0f * tex_scale_.x, tex_scroll_.y + 1.0f * tex_scale_.y },
					{ offset_.x - 0.5f * size_.x, offset_.y + 0.5f * size_.y, tex_scroll_.x + 0.0f * tex_scale_.x, tex_scroll_.y + 0.0f * tex_scale_.y },
					{ offset_.x + 0.5f * size_.x, offset_.y + 0.5f * size_.y, tex_scroll_.x + 1.0f * tex_scale_.x, tex_scroll_.y + 0.0f * tex_scale_.y }
				};
				
				glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, is_dynamic_draw_ ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
			}
				
			render_data_.vertex_type = GL_TRIANGLE_STRIP;
		}
		
		render_data_.vertex_count = 4;
		
		if (is_use_tex2_)
			render_data_.vertex_format = POS_TEX2_2;
		else
			render_data_.vertex_format = POS_TEX_2;
	}
	
	void SpriteActor::SetSizeOffset(float width, float height, float offset_width /*= 0.0f*/, float offset_height /*= 0.0f*/)
	{
		size_.x = width;
		size_.y = height;
		offset_.x = offset_width;
		offset_.y = offset_height;
		
		UpdateVertexBuffer();
	}
	
	void SpriteActor::SetTexScale(float u_scale, float v_scale, bool is_tex2 /*= false*/)
	{
		if (is_tex2)
		{
			tex_scale2_.x = u_scale;
			tex_scale2_.y = v_scale;
			is_use_tex2_ = true;
		}
		else
		{
			tex_scale_.x = u_scale;
			tex_scale_.y = v_scale;
		}
		
		UpdateVertexBuffer();
	}
	
	void SpriteActor::SetTexScroll(float u_scroll, float v_scroll, bool is_tex2 /*= false*/)
	{
		if (is_tex2)
		{
			tex_scroll2_.x = u_scroll;
			tex_scroll2_.y = v_scroll;
			is_use_tex2_ = true;
		}
		else
		{
			tex_scroll_.x = u_scroll;
			tex_scroll_.y = v_scroll;
		}
		
		UpdateVertexBuffer();
	}

	void SpriteActor::SetTexArea(int start_x, int start_y, int width, int height, bool is_tex2 /*= false*/)
	{
		const Texture* tex = is_tex2 ? material_data_.texture_units[1].texture : material_data_.texture_units[0].texture;
		
		ASSERT(tex);
		
		SetTexScale(static_cast<float>(width) / tex->width, static_cast<float>(height) / tex->height, is_tex2);
		SetTexScroll(static_cast<float>(start_x) / tex->width, static_cast<float>(start_y) / tex->height, is_tex2);
	}
	
	void SpriteActor::SetTexAreaUV(float start_u, float start_v, float width, float height, bool is_tex2 /*= false*/)
	{
		SetTexScale(width, height, is_tex2);
		SetTexScroll(start_u, start_v, is_tex2);
	}

	void SpriteActor::SetTxt(const std::string& txt, const std::string& font_name, float font_size)
	{
		const Texture* tex = Root::Ins().texture_mgr()->GetTxtTexture(txt, font_name, font_size, size_.x, size_.y);
		
		ASSERT(tex);
		
		SetMaterial(tex);
	}

	void SpriteActor::SetUseLine(bool use_line)
	{
		if (is_use_line_ != use_line)
		{
			is_use_line_ = use_line;
			
			UpdateVertexBuffer();
		}
	}
	
	bool SpriteActor::IsInArea(const Vector3& local_space_pos)
	{
		if (local_space_pos.x >= (offset_.x - 0.5f * size_.x - area_border_)
			&& local_space_pos.x <= (offset_.x + 0.5f * size_.x + area_border_)
			&& local_space_pos.y >= (offset_.y - 0.5f * size_.y - area_border_)
			&& local_space_pos.y <= (offset_.y + 0.5f * size_.y + area_border_))
		{
			return true;
		}
	
		return false;
	}
	
#pragma mark BoxActor
	
	BoxActor::BoxActor(const Vector3& half_ext) : half_ext_(half_ext)
	{
		UpdateVertexBuffer();
	}
	
	BoxActor::~BoxActor()
	{
	}
	
	void BoxActor::UpdateVertexBuffer()
	{
		if (render_data_.vertex_buffer == 0)
		{
			glGenBuffers(1, &render_data_.vertex_buffer);
		}
		
		Vector2 unit_uv_(1.0f, 1.0f);
		
		vertex_3_pos_normal_tex v[36] = {
			
			// front
			{ - half_ext_.x, - half_ext_.y, + half_ext_.z, 0, 0, 1, 0.0f * unit_uv_.x, 1.0f * unit_uv_.y },
			{ + half_ext_.x, - half_ext_.y, + half_ext_.z, 0, 0, 1, 1.0f * unit_uv_.x, 1.0f * unit_uv_.y },
			{ - half_ext_.x, + half_ext_.y, + half_ext_.z, 0, 0, 1, 0.0f * unit_uv_.x, 0.0f * unit_uv_.y },
			{ - half_ext_.x, + half_ext_.y, + half_ext_.z, 0, 0, 1, 0.0f * unit_uv_.x, 0.0f * unit_uv_.y },
			{ + half_ext_.x, - half_ext_.y, + half_ext_.z, 0, 0, 1, 1.0f * unit_uv_.x, 1.0f * unit_uv_.y },
			{ + half_ext_.x, + half_ext_.y, + half_ext_.z, 0, 0, 1, 1.0f * unit_uv_.x, 0.0f * unit_uv_.y },

			// back
			{ + half_ext_.x, - half_ext_.y, - half_ext_.z, 0, 0, -1, 0.0f * unit_uv_.x, 1.0f * unit_uv_.y },
			{ - half_ext_.x, - half_ext_.y, - half_ext_.z, 0, 0, -1, 1.0f * unit_uv_.x, 1.0f * unit_uv_.y },
			{ + half_ext_.x, + half_ext_.y, - half_ext_.z, 0, 0, -1, 0.0f * unit_uv_.x, 0.0f * unit_uv_.y },
			{ + half_ext_.x, + half_ext_.y, - half_ext_.z, 0, 0, -1, 0.0f * unit_uv_.x, 0.0f * unit_uv_.y },
			{ - half_ext_.x, - half_ext_.y, - half_ext_.z, 0, 0, -1, 1.0f * unit_uv_.x, 1.0f * unit_uv_.y },
			{ - half_ext_.x, + half_ext_.y, - half_ext_.z, 0, 0, -1, 1.0f * unit_uv_.x, 0.0f * unit_uv_.y },
			
			// top
			{ - half_ext_.x, + half_ext_.y, + half_ext_.z, 0, 1, 0, 0.0f * unit_uv_.x, 1.0f * unit_uv_.y },
			{ + half_ext_.x, + half_ext_.y, + half_ext_.z, 0, 1, 0, 1.0f * unit_uv_.x, 1.0f * unit_uv_.y },
			{ - half_ext_.x, + half_ext_.y, - half_ext_.z, 0, 1, 0, 0.0f * unit_uv_.x, 0.0f * unit_uv_.y },
			{ - half_ext_.x, + half_ext_.y, - half_ext_.z, 0, 1, 0, 0.0f * unit_uv_.x, 0.0f * unit_uv_.y },
			{ + half_ext_.x, + half_ext_.y, + half_ext_.z, 0, 1, 0, 1.0f * unit_uv_.x, 1.0f * unit_uv_.y },
			{ + half_ext_.x, + half_ext_.y, - half_ext_.z, 0, 1, 0, 1.0f * unit_uv_.x, 0.0f * unit_uv_.y },
			
			// bottom
			{ - half_ext_.x, - half_ext_.y, - half_ext_.z, 0, -1, 0, 0.0f * unit_uv_.x, 1.0f * unit_uv_.y },
			{ + half_ext_.x, - half_ext_.y, - half_ext_.z, 0, -1, 0, 1.0f * unit_uv_.x, 1.0f * unit_uv_.y },
			{ - half_ext_.x, - half_ext_.y, + half_ext_.z, 0, -1, 0, 0.0f * unit_uv_.x, 0.0f * unit_uv_.y },
			{ - half_ext_.x, - half_ext_.y, + half_ext_.z, 0, -1, 0, 0.0f * unit_uv_.x, 0.0f * unit_uv_.y },
			{ + half_ext_.x, - half_ext_.y, - half_ext_.z, 0, -1, 0, 1.0f * unit_uv_.x, 1.0f * unit_uv_.y },
			{ + half_ext_.x, - half_ext_.y, + half_ext_.z, 0, -1, 0, 1.0f * unit_uv_.x, 0.0f * unit_uv_.y },
			
			// right
			{ + half_ext_.x, - half_ext_.y, + half_ext_.z, 1, 0, 0, 0.0f * unit_uv_.x, 1.0f * unit_uv_.y },
			{ + half_ext_.x, - half_ext_.y, - half_ext_.z, 1, 0, 0, 1.0f * unit_uv_.x, 1.0f * unit_uv_.y },
			{ + half_ext_.x, + half_ext_.y, + half_ext_.z, 1, 0, 0, 0.0f * unit_uv_.x, 0.0f * unit_uv_.y },
			{ + half_ext_.x, + half_ext_.y, + half_ext_.z, 1, 0, 0, 0.0f * unit_uv_.x, 0.0f * unit_uv_.y },
			{ + half_ext_.x, - half_ext_.y, - half_ext_.z, 1, 0, 0, 1.0f * unit_uv_.x, 1.0f * unit_uv_.y },
			{ + half_ext_.x, + half_ext_.y, - half_ext_.z, 1, 0, 0, 1.0f * unit_uv_.x, 0.0f * unit_uv_.y },

			// left
			{ - half_ext_.x, - half_ext_.y, - half_ext_.z, -1, 0, 0, 0.0f * unit_uv_.x, 1.0f * unit_uv_.y },
			{ - half_ext_.x, - half_ext_.y, + half_ext_.z, -1, 0, 0, 1.0f * unit_uv_.x, 1.0f * unit_uv_.y },
			{ - half_ext_.x, + half_ext_.y, - half_ext_.z, -1, 0, 0, 0.0f * unit_uv_.x, 0.0f * unit_uv_.y },
			{ - half_ext_.x, + half_ext_.y, - half_ext_.z, -1, 0, 0, 0.0f * unit_uv_.x, 0.0f * unit_uv_.y },
			{ - half_ext_.x, - half_ext_.y, + half_ext_.z, -1, 0, 0, 1.0f * unit_uv_.x, 1.0f * unit_uv_.y },
			{ - half_ext_.x, + half_ext_.y, + half_ext_.z, -1, 0, 0, 1.0f * unit_uv_.x, 0.0f * unit_uv_.y },
		};
		
		render_data_.vertex_type = GL_TRIANGLES;
		render_data_.vertex_format = POS_NORMAL_TEX_3;
		render_data_.vertex_count = 36;
		
		glBindBuffer(GL_ARRAY_BUFFER, render_data_.vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
	}

#pragma mark NumberActor
	
	NumberActor::NumberActor(float width, float height, bool is_force_sign /*= true*/) :
		vertices_(NULL),
		now_len_max_(0),
		now_len_(0),
		size_(Vector2(width, height)),
		unit_uv_(Vector2(1.0f, 1.0f)),
		is_use_line_(false),
		number_(0),
		number_f_(0.0f),
		is_float_(false),
		is_force_sign_(is_force_sign)
	{
	}
	
	NumberActor::NumberActor(float width, float height, const std::string& material, int tex_unit_width, int tex_unit_height, bool is_force_sign /*= true*/) :
		vertices_(NULL),
		now_len_max_(0),
		now_len_(0),
		size_(Vector2(width, height)),
		is_use_line_(false),
		number_(0),
		number_f_(0.0f),
		is_float_(false),
		is_force_sign_(is_force_sign)
	{
		SetMaterial(material);
		SetTexUnit(tex_unit_width, tex_unit_height);
	}
	
	NumberActor::~NumberActor()
	{
		if (vertices_) free(vertices_);
	}
	
	void NumberActor::SetTexUnit(int tex_unit_width, int tex_unit_height)
	{
		const Texture* tex = material_data_.texture_units[0].texture;
		
		ASSERT(tex);
		
		unit_uv_.x = static_cast<float>(tex_unit_width) / tex->width;
		unit_uv_.y = static_cast<float>(tex_unit_height) / tex->height;
		
		if (vertices_)
		{
			UpdateVertexBuffer();
		}
	}
	
	void NumberActor::SetUseLine(bool use_line)
	{
		if (is_use_line_ != use_line)
		{
			is_use_line_ = use_line;
			
			if (vertices_)
			{
				UpdateVertexBuffer();
			}
		}
	}
	
	void NumberActor::SetNumber(int number)
	{
		number_ = number;
		is_float_ = false;
		
		UpdateVertexBuffer();
	}
	
	void NumberActor::SetNumberFloat(float number)
	{
		is_float_ = true;
		number_f_ = number;
		
		UpdateVertexBuffer();
	}
	
	bool NumberActor::IsInArea(const Vector3& local_space_pos)
	{
		int len = render_data_.vertex_count / 6;
		
		if (local_space_pos.x >= (size_.x * ((len - 1) * (-0.5f) - 0.5f))
			&& local_space_pos.x <= (size_.x * ((len - 1) * 0.5f + 0.5f))
			&& local_space_pos.y >= (-0.5f * size_.y)
			&& local_space_pos.y <= (0.5f * size_.y))
		{
			return true;
		}
		
		return false;
	}
	
	void NumberActor::UpdateVertexBuffer()
	{
		char number_str[16];
		
		if (is_float_)
			sprintf(number_str, (!is_force_sign_ || number_ == 0) ? "%.2f" : "%+.2f", number_f_);
		else
			sprintf(number_str, (!is_force_sign_ || number_ == 0) ? "%d" : "%+d", number_);
		
		now_len_ = strlen(number_str);
		
		int unit_vertex_num = is_use_line_ ? 8 : 6;
		
		if (now_len_max_ < now_len_)
		{
			now_len_max_ = (now_len_ >= 8) ? 16 : 8;
			
			ASSERT(now_len_max_ >= now_len_);
			
			if (vertices_) free(vertices_);
			vertices_ = static_cast<vertex_2_pos_tex*>(malloc(now_len_max_ * unit_vertex_num * sizeof(vertex_2_pos_tex)));
			
			if (render_data_.vertex_buffer != 0)
			{
				glDeleteBuffers(1, &render_data_.vertex_buffer);
			}
			
			glGenBuffers(1, &render_data_.vertex_buffer);
		}
		
		float start_x = (now_len_ - 1) * size_.x * -0.5f;
		int start_idx = 0;
		float scroll_u, scroll_v;
		scroll_u = scroll_v = 0;
		
		for (int i = 0; i < now_len_; ++i)
		{
			if (number_str[i] >= '0' && number_str[i] <= '9')
			{
				scroll_u = (number_str[i] - '0') * unit_uv_.x;
			}
			else if (number_str[i] == '+')
			{
				scroll_u = 10 * unit_uv_.x;
			}
			else if (number_str[i] == '-')
			{
				scroll_u = 11 * unit_uv_.x;
			}
			else if (number_str[i] == '.')
			{
				scroll_u = 12 * unit_uv_.x;
			}
			else
			{
				ASSERT(0);
			}
			
			if (is_use_line_)
			{
				vertex_2_pos_tex v[] = {
					{ start_x - 0.5f * size_.x, - 0.5f * size_.y, scroll_u + 0.0f * unit_uv_.x, scroll_v + 1.0f * unit_uv_.y },
					{ start_x + 0.5f * size_.x, - 0.5f * size_.y, scroll_u + 1.0f * unit_uv_.x, scroll_v + 1.0f * unit_uv_.y },
					{ start_x + 0.5f * size_.x, - 0.5f * size_.y, scroll_u + 1.0f * unit_uv_.x, scroll_v + 1.0f * unit_uv_.y },
					{ start_x + 0.5f * size_.x, + 0.5f * size_.y, scroll_u + 1.0f * unit_uv_.x, scroll_v + 0.0f * unit_uv_.y },
					{ start_x + 0.5f * size_.x, + 0.5f * size_.y, scroll_u + 1.0f * unit_uv_.x, scroll_v + 0.0f * unit_uv_.y },
					{ start_x - 0.5f * size_.x, + 0.5f * size_.y, scroll_u + 0.0f * unit_uv_.x, scroll_v + 0.0f * unit_uv_.y },
					{ start_x - 0.5f * size_.x, + 0.5f * size_.y, scroll_u + 0.0f * unit_uv_.x, scroll_v + 0.0f * unit_uv_.y },
					{ start_x - 0.5f * size_.x, - 0.5f * size_.y, scroll_u + 0.0f * unit_uv_.x, scroll_v + 1.0f * unit_uv_.y }
				};
				
				memcpy(&vertices_[start_idx], v, sizeof(v));
				
			}
			else
			{
				vertex_2_pos_tex v[] = {
					{ start_x - 0.5f * size_.x, - 0.5f * size_.y, scroll_u + 0.0f * unit_uv_.x, scroll_v + 1.0f * unit_uv_.y },
					{ start_x + 0.5f * size_.x, - 0.5f * size_.y, scroll_u + 1.0f * unit_uv_.x, scroll_v + 1.0f * unit_uv_.y },
					{ start_x - 0.5f * size_.x, + 0.5f * size_.y, scroll_u + 0.0f * unit_uv_.x, scroll_v + 0.0f * unit_uv_.y },
					{ start_x - 0.5f * size_.x, + 0.5f * size_.y, scroll_u + 0.0f * unit_uv_.x, scroll_v + 0.0f * unit_uv_.y },
					{ start_x + 0.5f * size_.x, - 0.5f * size_.y, scroll_u + 1.0f * unit_uv_.x, scroll_v + 1.0f * unit_uv_.y },
					{ start_x + 0.5f * size_.x, + 0.5f * size_.y, scroll_u + 1.0f * unit_uv_.x, scroll_v + 0.0f * unit_uv_.y }
				};
				
				memcpy(&vertices_[start_idx], v, sizeof(v));
			}
			
			start_x += size_.x;
			start_idx += unit_vertex_num;
		}
		
		render_data_.vertex_count = now_len_ * unit_vertex_num;
		render_data_.vertex_type = is_use_line_ ? GL_LINES : GL_TRIANGLES;
		
		glBindBuffer(GL_ARRAY_BUFFER, render_data_.vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, render_data_.vertex_count * sizeof(vertex_2_pos_tex), vertices_, GL_DYNAMIC_DRAW);
	}
	
#pragma mark TxtActor
	
	TxtActor::TxtActor(const std::string& txt, const std::string& font_name, int font_size, bool is_pos_center /*= false*/) :
		vertices_(NULL),
		now_len_max_(0),
		now_len_(0),
		font_(NULL),
		font_size_(font_size),
		is_use_line_(false),
		width_(0.0f),
		height_(0.0f),
		area_border_(0.0f),
		is_pos_center_(is_pos_center),
		txt_(txt)
	{
		font_ = Root::Ins().font_mgr()->GetFont(font_name);
		
		ASSERT(font_);
		
		SetMaterial(font_->texture, font_->filter_min, font_->filter_mag);
		
		UpdateVertexBuffer();
	}
	
	TxtActor::~TxtActor()
	{
		if (vertices_) free(vertices_);
	}
	
	void TxtActor::SetTxt(const std::string& txt)
	{
		txt_ = txt;
		UpdateVertexBuffer();
	}
	
	void TxtActor::CalculateSize(const std::string& txt, const Font* font, int font_size, float& width, float& height)
	{
		ASSERT(font);
		
		float size_scale = static_cast<float>(font_size) / font->size;
		
		width = height = 0;
		
		int len = txt.length();
		
		if (len == 0)
			return;
		
		height = font->common_line_height * size_scale;
		
		float now_width = 0;
		for (int i = 0; i < len; ++i)
		{
			if (txt[i] == '\n')
			{
				if (now_width > width) width = now_width;
				now_width = 0;
				height += font->common_line_height * size_scale;
			}
			else
			{
				const CharSetting& setting = font->GetCharSetting(txt[i]);
				now_width += setting.x_advance * size_scale;
			}
		}
		if (now_width > width) width = now_width;
	}
	
	bool TxtActor::IsInArea(const Vector3& local_space_pos)
	{
		if (local_space_pos.x >= ((is_pos_center_ ? (-width_ / 2) : 0) - area_border_)
			&& local_space_pos.x <= ((is_pos_center_ ? (width_ / 2) : width_) + area_border_)
			&& local_space_pos.y >= ((is_pos_center_ ? (-height_ / 2) : -height_) - area_border_)
			&& local_space_pos.y <= ((is_pos_center_ ? (height_ / 2) : 0) + area_border_))
		{
			return true;
		}

		return false;
	}
	
	void TxtActor::UpdateVertexBuffer()
	{
		now_len_ = txt_.length();
		
		int unit_vertex_num = is_use_line_ ? 8 : 6;
		
		if (now_len_max_ < now_len_)
		{
			now_len_max_ = now_len_;
			
			if (vertices_) free(vertices_);
			vertices_ = static_cast<vertex_2_pos_tex*>(malloc(now_len_max_ * unit_vertex_num * sizeof(vertex_2_pos_tex)));
			
			if (render_data_.vertex_buffer != 0)
			{
				glDeleteBuffers(1, &render_data_.vertex_buffer);
			}
			
			glGenBuffers(1, &render_data_.vertex_buffer);
		}
		
		float inv_tex_width = 1.0f / font_->texture->width;
		float inv_tex_height = 1.0f / font_->texture->height;
		float size_scale = static_cast<float>(font_size_) / font_->size;
		
		CalculateSize(txt_, font_, font_size_, width_, height_);
		
		float start_x = is_pos_center_ ? width_ * -0.5f : 0;
		float start_y = is_pos_center_ ? height_ * 0.5f : 0;
		int start_idx = 0;
		float scroll_u, scroll_v, unit_u, unit_v, offset_x, offset_y, size_x, size_y;
		
		int invisible_num = 0;
		
		for (int i = 0; i < now_len_; ++i)
		{
			if (txt_[i] == '\n')
			{
				start_x = is_pos_center_ ? width_ * -0.5f : 0;
				start_y -= font_->common_line_height * size_scale;
				++invisible_num;
			}
			else
			{
				const CharSetting& setting = font_->GetCharSetting(txt_[i]);
				
				scroll_u = setting.x * inv_tex_width;
				scroll_v = setting.y * inv_tex_height;
				unit_u = (setting.width - 1) * inv_tex_width;
				unit_v = (setting.height - 1) * inv_tex_height;
				
				offset_x = setting.x_offset * size_scale;
				offset_y = setting.y_offset * size_scale;
				size_x = (setting.width - 1) * size_scale;
				size_y = (setting.height - 1) * size_scale;
				
				if (is_use_line_)
				{
					vertex_2_pos_tex v[] = {
						{ start_x + offset_x,			start_y - offset_y - size_y,	scroll_u + 0.0f * unit_u, scroll_v + 1.0f * unit_v },
						{ start_x + offset_x + size_x,	start_y - offset_y - size_y,	scroll_u + 1.0f * unit_u, scroll_v + 1.0f * unit_v },
						{ start_x + offset_x + size_x,	start_y - offset_y - size_y,	scroll_u + 1.0f * unit_u, scroll_v + 1.0f * unit_v },
						{ start_x + offset_x + size_x,	start_y - offset_y,				scroll_u + 1.0f * unit_u, scroll_v + 0.0f * unit_v },
						{ start_x + offset_x + size_x,	start_y - offset_y,				scroll_u + 1.0f * unit_u, scroll_v + 0.0f * unit_v },
						{ start_x + offset_x,			start_y - offset_y,				scroll_u + 0.0f * unit_u, scroll_v + 0.0f * unit_v },
						{ start_x + offset_x,			start_y - offset_y,				scroll_u + 0.0f * unit_u, scroll_v + 0.0f * unit_v },
						{ start_x + offset_x,			start_y - offset_y - size_y,	scroll_u + 0.0f * unit_u, scroll_v + 1.0f * unit_v }
					};
					
					memcpy(&vertices_[start_idx], v, sizeof(v));
				}
				else
				{
					vertex_2_pos_tex v[] = {
						{ start_x + offset_x,			start_y - offset_y - size_y,	scroll_u + 0.0f * unit_u, scroll_v + 1.0f * unit_v },
						{ start_x + offset_x + size_x,	start_y - offset_y - size_y,	scroll_u + 1.0f * unit_u, scroll_v + 1.0f * unit_v },
						{ start_x + offset_x,			start_y - offset_y,				scroll_u + 0.0f * unit_u, scroll_v + 0.0f * unit_v },
						{ start_x + offset_x,			start_y - offset_y,				scroll_u + 0.0f * unit_u, scroll_v + 0.0f * unit_v },
						{ start_x + offset_x + size_x,	start_y - offset_y - size_y,	scroll_u + 1.0f * unit_u, scroll_v + 1.0f * unit_v },
						{ start_x + offset_x + size_x,	start_y - offset_y,				scroll_u + 1.0f * unit_u, scroll_v + 0.0f * unit_v }
					};
					
					memcpy(&vertices_[start_idx], v, sizeof(v));
				}
				
				start_x += setting.x_advance * size_scale;
				start_idx += unit_vertex_num;
			}
		}
		
		render_data_.vertex_count = (now_len_ - invisible_num) * unit_vertex_num;
		render_data_.vertex_type = is_use_line_ ? GL_LINES : GL_TRIANGLES;
		
		glBindBuffer(GL_ARRAY_BUFFER, render_data_.vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, render_data_.vertex_count * sizeof(vertex_2_pos_tex), vertices_, GL_DYNAMIC_DRAW);
	}

}
