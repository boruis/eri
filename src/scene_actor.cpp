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

#include "root.h"
#include "render_data.h"
#include "renderer.h"
#include "texture_mgr.h"
#include "scene_mgr.h"

namespace ERI {
	
#pragma mark SceneActor
	
	SceneActor::SceneActor() :
		layer_(NULL),
		parent_(NULL),
		visible_(true),
		inherit_visible_(true),
		is_view_depth_dirty_(true),
		user_data_(NULL),
		bounding_sphere_(NULL),
		bounding_sphere_world_(NULL)
	{
	}
	
	SceneActor::~SceneActor()
	{
		if (bounding_sphere_)
		{
			delete bounding_sphere_;
			delete bounding_sphere_world_;
		}
		
		for (size_t i = 0; i < childs_.size(); ++i)
		{
			// TODO: is delete childs right?

			childs_[i]->parent_ = NULL;
			delete childs_[i];
		}
		
		if (parent_)
		{
			RemoveFromParent();
		}
		
		if (layer_)
		{
			RemoveFromScene();
		}
	}
	
	void SceneActor::AddToScene(int layer_id /*= 0*/)
	{
		ASSERT(!layer_);
			
		Root::Ins().scene_mgr()->AddActor(this, layer_id);
	}
	
	void SceneActor::RemoveFromScene()
	{
		ASSERT(layer_);
		
		Root::Ins().scene_mgr()->RemoveActor(this, layer_->id());
	}
	
	void SceneActor::MoveToLayer(int layer_id)
	{
		RemoveFromScene();
		AddToScene(layer_id);
	}
	
	void SceneActor::AddChild(SceneActor* actor)
	{
		ASSERT(actor && this != actor);
		
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
	
	void SceneActor::RemoveFromParent()
	{
		ASSERT(parent_);
		
		parent_->RemoveChild(this);
	}
	
	bool SceneActor::IsHit(const Vector3& world_space_pos)
	{
		return IsInArea(GetLocalSpacePos(world_space_pos));
	}
	
	SceneActor* SceneActor::GetHitActor(const Vector3& parent_space_pos)
	{
		if (!visible())
			return NULL;
		
		if (render_data_.need_update_model_matrix)
			render_data_.UpdateModelMatrix();

		Vector3 local_space_pos = render_data_.inv_model_matrix * parent_space_pos;
		
		if (IsInArea(local_space_pos))
			return this;

		if (!childs_.empty())
		{
			SceneActor* actor;
			for (int i = static_cast<int>(childs_.size()) - 1; i >= 0; --i)
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
		if (!visible())
			return;
		
		if (!IsInFrustum())
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
	
	const Color& SceneActor::GetColor() const
	{
		return render_data_.color;
	}
	
	void SceneActor::BlendNormal()
	{
		render_data_.blend_src_factor = GL_SRC_ALPHA;
		render_data_.blend_dst_factor = GL_ONE_MINUS_SRC_ALPHA;
	}
	
	void SceneActor::BlendAdd()
	{
		render_data_.blend_src_factor = GL_SRC_ALPHA;
		render_data_.blend_dst_factor = GL_ONE;
	}
	
	void SceneActor::BlendMultiply()
	{
		render_data_.blend_src_factor = GL_DST_COLOR;
		render_data_.blend_dst_factor = GL_ZERO;
	}
	
	void SceneActor::BlendMultiply2x()
	{
		render_data_.blend_src_factor = GL_DST_COLOR;
		render_data_.blend_dst_factor = GL_SRC_COLOR;
	}
	
	void SceneActor::BlendReplace()
	{
		render_data_.blend_src_factor = GL_ONE;
		render_data_.blend_dst_factor = GL_ZERO;
	}

	void SceneActor::AlphaTestGreater(int alpha_value)
	{
		render_data_.alpha_test_func = GL_GREATER;
		render_data_.alpha_test_ref = alpha_value / 255.0f;
	}

	void SceneActor::DepthTestEqual()
	{
		render_data_.depth_test_func = GL_EQUAL;
	}

	void SceneActor::DepthTestAlways()
	{
		render_data_.depth_test_func = GL_ALWAYS;
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
				render_data_.UpdateWorldModelMatrix(parent_->GetWorldTransform());
			else
				render_data_.UpdateWorldModelMatrix();
			
			if (bounding_sphere_)
				bounding_sphere_world_->center = render_data_.world_model_matrix * bounding_sphere_->center;
		}
		
		return render_data_.world_model_matrix;
	}
	
	Vector3 SceneActor::GetLocalSpacePos(const Vector3& world_space_pos)
	{
		std::vector<SceneActor*> parent_list;
		SceneActor* now_parent = parent_;
		while (now_parent)
		{
			parent_list.push_back(now_parent);
			now_parent = now_parent->parent_;
		}
		
		Vector3 local_space_pos = world_space_pos;
		
		for (int i = static_cast<int>(parent_list.size()) - 1; i >= 0; --i)
		{
			local_space_pos = parent_list[i]->GetInvTransform() * local_space_pos;
		}
		
		if (render_data_.need_update_model_matrix)
			render_data_.UpdateModelMatrix();
		
		local_space_pos = render_data_.inv_model_matrix * local_space_pos;
		
		return local_space_pos;
	}
	
	void SceneActor::SetPos(float x, float y)
	{
		render_data_.translate.x = x;
		render_data_.translate.y = y;
		SetTransformDirty();
	}
	
	Vector2 SceneActor::GetPos() const
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
	
	Vector2 SceneActor::GetScale() const
	{
		return Vector2(render_data_.scale.x, render_data_.scale.y);
	}
	
	void SceneActor::SetPos(const Vector3& pos)
	{
		render_data_.translate = pos;
		SetTransformDirty();
	}
	
	const Vector3& SceneActor::GetPos3() const
	{
		return render_data_.translate;
	}
	
	void SceneActor::SetRotate(float degree, const Vector3& axis)
	{
		render_data_.rotate_degree = degree;
		render_data_.rotate_axis = axis;
		SetTransformDirty();
	}
	
	void SceneActor::GetRotate(float& out_degree, Vector3& out_axis) const
	{
		out_degree = render_data_.rotate_degree;
		out_axis = render_data_.rotate_axis;
	}
	
	void SceneActor::SetScale(const Vector3& scale)
	{
		render_data_.scale = scale;
		SetTransformDirty();
	}
	
	const Vector3& SceneActor::GetScale3() const
	{
		return render_data_.scale;
	}
	
	float SceneActor::GetViewDepth()
	{
		if (is_view_depth_dirty_)
		{
			render_data_.world_view_pos = GetWorldTransform().GetTranslate();
			
			// TODO: should multiply view matrix in 3d view
			
			is_view_depth_dirty_ = false;
		}

		return render_data_.world_view_pos.z;
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
	
	void SceneActor::SetTextureWrap(TextureWrap wrap_s, TextureWrap wrap_t)
	{
		ASSERT(material_data_.used_unit > 0);
		
		material_data_.texture_units[0].params.wrap_s = wrap_s;
		material_data_.texture_units[0].params.wrap_t = wrap_t;
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
			if (layer_)
			{
				int layer_id = layer_->id();
				RemoveFromScene();
				material_data_.opacity_type = type;
				AddToScene(layer_id);
			}
			else
			{
				material_data_.opacity_type = type;
			}
		}
	}
	
	void SceneActor::SetDepthTest(bool enable)
	{
		// TODO: actually, disable depth test will disable depth write 
		
		material_data_.depth_test = enable;
	}
	
	void SceneActor::SetDepthWrite(bool enable)
	{
		// TODO: actually, disable depth test will disable depth write
		
		material_data_.depth_write = enable;
	}

	void SceneActor::SetCullFace(bool enable)
	{
		material_data_.cull_face = enable;
	}
	
	void SceneActor::SetColorWrite(bool r_enable, bool g_enable, bool b_enable, bool a_enable)
	{
		material_data_.color_write.r = r_enable;
		material_data_.color_write.g = g_enable;
		material_data_.color_write.b = b_enable;
		material_data_.color_write.a = a_enable;
	}
	
	void SceneActor::SetVisible(bool visible, bool inherit /*= false*/)
	{
		bool original_visible = visible_ && inherit_visible_;
		
		if (inherit)
			inherit_visible_ = visible;
		else
			visible_ = visible;
		
		bool current_visible = visible_ && inherit_visible_;
				
		if (current_visible != original_visible)
		{
			int child_num = childs_.size();
			for (int i = 0; i < child_num; ++i)
			{
				childs_[i]->SetVisible(current_visible, true);
			}
		}
	}
	
	bool SceneActor::IsInFrustum()
	{
		if (layer_ && bounding_sphere_world_)
		{
			GetWorldTransform();
			
			// TODO: no default camera cause no frustum culling, fix it
			
			CameraActor* cam = layer_->cam();
			if (!cam) cam = Root::Ins().scene_mgr()->default_cam();
			
			if (cam) return cam->IsInFrustum(bounding_sphere_world_);
		}
		
		return true;
	}
	
	void SceneActor::SetTransformDirty()
	{
		render_data_.need_update_model_matrix = true;

		SetWorldTransformDirty();
	}
	
	void SceneActor::SetWorldTransformDirty()
	{
		render_data_.need_update_world_model_matrix = true;
		
		size_t child_num = childs_.size();
		for (int i = 0; i < child_num; ++i)
		{
			childs_[i]->SetWorldTransformDirty();
		}
		
		is_view_depth_dirty_ = true;
		if (layer_) layer_->SetSortDirty();
	}
	
	void SceneActor::SetTexture(int idx, const Texture* tex)
	{
		if (material_data_.texture_units[idx].texture != tex)
		{
			if (layer_)
			{
				int original_texture_id = material_data_.GetSingleTextureId();
				material_data_.texture_units[idx].texture = tex;
				layer_->AdjustActorMaterial(this, original_texture_id);
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
		far_z_(1000.0f),
		is_view_modified_(true),
		is_projection_modified_(true),
		is_view_need_update_(true),
		is_projection_need_update_(true),
		is_frustum_dirty_(true)
	{
	}
	
	CameraActor::~CameraActor()
	{
	}
	
	void CameraActor::SetPos(float x, float y)
	{
		SceneActor::SetPos(x, y);
		
		SetViewModified();
	}
	
	void CameraActor::SetPos(const Vector3& pos)
	{
		SceneActor::SetPos(pos);
		
		SetViewModified();
	}
	
	void CameraActor::SetLookAt(const Vector3& look_at, bool is_offset)
	{
		look_at_ = look_at;
		is_look_at_offset_ = is_offset;

		SetViewModified();
		
		// TODO: modify self rotation to make childs' transform correct
	}
	
	void CameraActor::SetOrthoZoom(float zoom)
	{
		ASSERT(projection_ == ORTHOGONAL);
		ASSERT(zoom > 0);
		
		ortho_zoom_ = zoom;
		SetScale(Vector3(1 / ortho_zoom_, 1 / ortho_zoom_, 1 / ortho_zoom_));
		//SetScale(1 / ortho_zoom_, 1 / ortho_zoom_);
		
		SetProjectionModified();
	}
	
	void CameraActor::SetPerspectiveFov(float fov_y)
	{
		ASSERT(projection_ == PERSPECTIVE);
		ASSERT(fov_y > 0);
		
		perspective_fov_y_ = fov_y;
		
		SetProjectionModified();
	}
	
	void CameraActor::SetFarZ(float far_z)
	{
		ASSERT(far_z > 0);
		
		far_z_ = far_z;
		
		SetProjectionModified();
	}
	
	void CameraActor::UpdateViewMatrix()
	{
		ASSERT(is_view_need_update_);
		
		if (is_view_modified_)
			CalculateViewMatrix();
		
		Root::Ins().renderer()->UpdateView(view_matrix_);
		
		is_view_need_update_ = false;
	}
	
	void CameraActor::UpdateProjectionMatrix()
	{
		ASSERT(is_projection_need_update_);

		if (is_projection_modified_)
			CalculateProjectionMatrix();
		
		Root::Ins().renderer()->UpdateProjection(projection_matrix_);
		
		is_projection_need_update_ = false;
	}
	
	void CameraActor::SetViewModified()
	{
		is_view_modified_ = true;
		is_view_need_update_ = true;
		is_frustum_dirty_ = true;
	}
	
	void CameraActor::SetProjectionModified()
	{
		is_projection_modified_ = true;
		is_projection_need_update_ = true;
		is_frustum_dirty_ = true;
	}
	
	void CameraActor::SetViewProjectionNeedUpdate()
	{
		is_view_need_update_ = true;
		is_projection_need_update_ = true;
	}
	
	bool CameraActor::IsInFrustum(const Sphere* sphere)
	{
		if (is_frustum_dirty_)
		{
			if (is_view_modified_)
				CalculateViewMatrix();
			
			if (is_projection_modified_)
				CalculateProjectionMatrix();
			
			ExtractFrustum(view_matrix_, projection_matrix_, frustum_);
			
			is_frustum_dirty_ = false;
		}
		
		return SphereInFrustum(*sphere, frustum_) > 0.0f;
	}
	
	void CameraActor::CalculateViewMatrix()
	{
		ASSERT(is_view_modified_);
		
		const Vector3& pos = GetPos3();
		MatrixLookAtRH(view_matrix_, pos, is_look_at_offset_ ? (pos + look_at_) : look_at_, Vector3(0, 1, 0));
		
		is_view_modified_ = false;
	}
	
	void CameraActor::CalculateProjectionMatrix()
	{
		ASSERT(is_projection_modified_);
		
		Renderer* renderer = Root::Ins().renderer();
		
		if (projection_ == ORTHOGONAL)
		{
			MatrixOrthoRH(projection_matrix_,
						  renderer->backing_width() / ortho_zoom_,
						  renderer->backing_height() / ortho_zoom_,
						  -far_z_, far_z_);
		}
		else
		{
			MatrixPerspectiveFovRH(projection_matrix_,
								   perspective_fov_y_,
								   static_cast<float>(renderer->backing_width()) / renderer->backing_height(),
								   1, far_z_);
		}
		
		is_projection_modified_ = false;
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
		
		float u_scale = static_cast<float>(width) / tex->width;
		float v_scale = static_cast<float>(height) / tex->height;
		float u_scroll = static_cast<float>(start_x) / tex->width;
		float v_scroll = static_cast<float>(start_y) / tex->height;
		
		if (is_tex2)
		{
			tex_scale2_.x = u_scale;
			tex_scale2_.y = v_scale;
			tex_scroll2_.x = u_scroll;
			tex_scroll2_.y = v_scroll;
			is_use_tex2_ = true;
		}
		else
		{
			tex_scale_.x = u_scale;
			tex_scale_.y = v_scale;
			tex_scroll_.x = u_scroll;
			tex_scroll_.y = v_scroll;
		}
		
		UpdateVertexBuffer();
	}
	
	void SpriteActor::SetTexAreaUV(float start_u, float start_v, float width, float height, bool is_tex2 /*= false*/)
	{
		if (is_tex2)
		{
			tex_scale2_.x = width;
			tex_scale2_.y = height;
			tex_scroll2_.x = start_u;
			tex_scroll2_.y = start_v;
			is_use_tex2_ = true;
		}
		else
		{
			tex_scale_.x = width;
			tex_scale_.y = height;
			tex_scroll_.x = start_u;
			tex_scroll_.y = start_v;
		}
		
		UpdateVertexBuffer();
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
	
	void SpriteActor::CreateBounding()
	{
		if (!bounding_sphere_)
			bounding_sphere_ = new Sphere;

		bounding_sphere_->center = Vector3(offset_);
		bounding_sphere_->radius = size_.Length() * 0.5f;
		
		if (!bounding_sphere_world_)
			bounding_sphere_world_ = new Sphere;

		bounding_sphere_world_->radius = bounding_sphere_->radius;
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
	
	NumberActor::NumberActor(float width, float height, bool is_force_sign /*= false*/) :
		vertices_(NULL),
		now_len_max_(0),
		now_len_(0),
		size_(Vector2(width, height)),
		tex_unit_uv_(Vector2(1.0f, 1.0f)),
		tex_scale_(Vector2(1.0f, 1.0f)),
		spacing_(0.0f),
		number_(0),
		number_f_(0.0f),
		is_float_(false),
		is_force_sign_(is_force_sign)
	{
	}
	
	NumberActor::~NumberActor()
	{
		if (vertices_) free(vertices_);
	}
	
	void NumberActor::SetTexUnit(int width, int height)
	{
		const Texture* tex = material_data_.texture_units[0].texture;
		
		ASSERT(tex);
		
		tex_unit_uv_.x = static_cast<float>(width) / tex->width;
		tex_unit_uv_.y = static_cast<float>(height) / tex->height;
		
		if (vertices_)
		{
			UpdateVertexBuffer();
		}
	}
	
	void NumberActor::SetTexArea(int start_x, int start_y, int width, int height)
	{
		const Texture* tex = material_data_.texture_units[0].texture;
		
		ASSERT(tex);
		
		tex_scale_.x = static_cast<float>(width) / tex->width;
		tex_scale_.y = static_cast<float>(height) / tex->height;
		tex_scroll_.x = static_cast<float>(start_x) / tex->width;
		tex_scroll_.y = static_cast<float>(start_y) / tex->height;
		
		if (vertices_)
		{
			UpdateVertexBuffer();
		}
	}
	
	void NumberActor::SetSpacing(float spacing)
	{
		spacing_ = spacing;
		
		if (vertices_)
		{
			UpdateVertexBuffer();
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
			sprintf(number_str, (!is_force_sign_ || number_f_ == 0.0f) ? "%.2f" : "%+.2f", number_f_);
		else
			sprintf(number_str, (!is_force_sign_ || number_ == 0) ? "%d" : "%+d", number_);
		
		now_len_ = static_cast<int>(strlen(number_str));
		
		int unit_vertex_num = 6;
		
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
		
		float start_x = (now_len_ - 1) * (size_.x + spacing_) * -0.5f;
		int start_idx = 0;
		float scroll_u, scroll_v;
		
		for (int i = 0; i < now_len_; ++i)
		{
			scroll_u = tex_scroll_.x;
			scroll_v = tex_scroll_.y;
			
			if (number_str[i] >= '0' && number_str[i] <= '9')
			{
				scroll_u += (number_str[i] - '0') * tex_unit_uv_.x;
			}
			else if (number_str[i] == '+')
			{
				scroll_u += 10 * tex_unit_uv_.x;
			}
			else if (number_str[i] == '-')
			{
				scroll_u += 11 * tex_unit_uv_.x;
			}
			else if (number_str[i] == '.')
			{
				scroll_u += 12 * tex_unit_uv_.x;
			}
			else
			{
				ASSERT(0);
			}
			
			vertex_2_pos_tex v[] = {
				{ start_x - 0.5f * size_.x, - 0.5f * size_.y, scroll_u + 0.0f * tex_unit_uv_.x, scroll_v + 1.0f * tex_unit_uv_.y },
				{ start_x + 0.5f * size_.x, - 0.5f * size_.y, scroll_u + 1.0f * tex_unit_uv_.x, scroll_v + 1.0f * tex_unit_uv_.y },
				{ start_x - 0.5f * size_.x, + 0.5f * size_.y, scroll_u + 0.0f * tex_unit_uv_.x, scroll_v + 0.0f * tex_unit_uv_.y },
				{ start_x - 0.5f * size_.x, + 0.5f * size_.y, scroll_u + 0.0f * tex_unit_uv_.x, scroll_v + 0.0f * tex_unit_uv_.y },
				{ start_x + 0.5f * size_.x, - 0.5f * size_.y, scroll_u + 1.0f * tex_unit_uv_.x, scroll_v + 1.0f * tex_unit_uv_.y },
				{ start_x + 0.5f * size_.x, + 0.5f * size_.y, scroll_u + 1.0f * tex_unit_uv_.x, scroll_v + 0.0f * tex_unit_uv_.y }
			};
			
			memcpy(&vertices_[start_idx], v, sizeof(v));
			
			start_x += size_.x + spacing_;
			start_idx += unit_vertex_num;
		}
		
		render_data_.vertex_count = now_len_ * unit_vertex_num;
		render_data_.vertex_type = GL_TRIANGLES;
		
		glBindBuffer(GL_ARRAY_BUFFER, render_data_.vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, render_data_.vertex_count * sizeof(vertex_2_pos_tex), vertices_, GL_DYNAMIC_DRAW);
	}

}
