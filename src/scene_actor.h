/*
 *  scene_actor.h
 *  eri
 *
 *  Created by exe on 11/29/09.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#ifndef ERI_SCENE_ACTOR_H
#define ERI_SCENE_ACTOR_H

#include <string>
#include <vector>

#if ERI_PLATFORM == ERI_PLATFORM_WIN
#include "GL/glew.h"
//#include "GL/wglew.h"
//#include "GLee.h"
#elif ERI_PLATFORM == ERI_PLATFORM_ANDROID
#include <GLES/gl.h>
#elif ERI_PLATFORM == ERI_PLATFORM_IOS
#import <OpenGLES/ES1/gl.h>
//#import <OpenGLES/ES2/gl.h>
#endif

#include "math_helper.h"
#include "render_data.h"
#include "material_data.h"

namespace ERI {
	
	class Renderer;
	class SceneMgr;
	class SceneLayer;
	
	struct UserData
	{
		virtual ~UserData() {}
	};
	
#pragma mark SceneActor
	
	class SceneActor
	{
	public:
		SceneActor();
		virtual ~SceneActor();
		
		// scene
		
		virtual void AddToScene(int layer_id = 0);
		virtual void RemoveFromScene();
		void MoveToLayer(int layer_id);
		
		void AddChild(SceneActor* actor);
		void RemoveChild(SceneActor* actor);
		void RemoveFromParent();
		
		// intersection
		
		bool IsHit(const Vector3& world_space_pos);
		SceneActor* GetHitActor(const Vector3& parent_space_pos);
		
		// render
				
		virtual void Render(Renderer* renderer);

		void SetColor(const Color& color);
		const Color& GetColor() const;
		
		void BlendNormal();
		void BlendAdd();
		void BlendMultiply();
		void BlendReplace();
		void AlphaTestGreater(int alpha_value);
		
		//
		// transform
		//
		
		const Matrix4& GetTransform();
		const Matrix4& GetInvTransform();
		const Matrix4& GetWorldTransform();
		Vector3 GetLocalSpacePos(const Vector3& world_space_pos);
		
		// TODO: 2D & 3D transform interface is suck, re-design
		
		// 2D
		virtual void SetPos(float x, float y);
		Vector2 GetPos() const;
				
		void SetRotate(float degree);
		float GetRotate() const;
		
		void SetScale(float x, float y);
		Vector2 GetScale() const;
		
		// 3D
		virtual void SetPos(const Vector3& pos);
		const Vector3& GetPos3() const;
		
		void SetRotate(float degree, const Vector3& axis);
		void GetRotate(float& out_degree, Vector3& out_axis) const;
		
		void SetScale(const Vector3& scale);
		const Vector3& GetScale3() const;
		
		//
		
		float GetViewDepth();
		
		// material
		// TODO: bad interface, re-design
		
		inline void set_accept_light(bool accept) { material_data_.accept_light = accept; }
		
		void SetMaterial(const std::string& texture_path, TextureFilter filter_min = FILTER_NEAREST, TextureFilter filter_mag = FILTER_NEAREST);
		void SetMaterial(const Texture* tex, TextureFilter filter_min = FILTER_NEAREST, TextureFilter filter_mag = FILTER_NEAREST);
		void AddMaterial(const std::string& texture_path, TextureFilter filter_min = FILTER_NEAREST, TextureFilter filter_mag = FILTER_NEAREST);
		void AddMaterial();
		
		void SetTextureFilter(TextureFilter filter_min, TextureFilter filter_mag);
		void SetTextureWrap(TextureWrap wrap_s, TextureWrap wrap_t);
		void SetTextureEnvs(int idx, const TextureEnvs& envs);
	
		void SetOpacityType(OpacityType type);
		inline OpacityType opacity_type() { return material_data_.opacity_type; }
		
		void SetDepthTest(bool enable);
		void SetDepthWrite(bool enable);

		inline const MaterialData& material() { return material_data_; }
		
		//

		inline void set_visible(bool visible) { visible_ = visible; }
		inline bool visible() { return visible_; }
		
		inline void set_user_data(UserData* data) { user_data_ = data; }
		inline UserData* user_data() { return user_data_; }
		
		inline SceneLayer* layer() { return layer_; }
		
		inline const SceneActor* parent() const { return parent_; }
		
		bool IsInFrustum();
		inline const Sphere* bounding_sphere() { return bounding_sphere_; }
		
		friend class SceneMgr;
			
	protected:
		virtual bool IsInArea(const Vector3& local_space_pos) { return false; }
		
		RenderData		render_data_;
		MaterialData	material_data_;

		SceneLayer*		layer_;
		
		SceneActor*					parent_;
		std::vector<SceneActor*>	childs_;

		bool			visible_;
		bool			is_view_depth_dirty_;
		
		UserData*		user_data_;
		
		Sphere*			bounding_sphere_;
		Sphere*			bounding_sphere_world_;
		
	private:
		void SetTransformDirty();
		void SetWorldTransformDirty();
		
		void SetTexture(int idx, const Texture* tex);
	};
	
#pragma mark CameraActor
	
	class CameraActor : public SceneActor
	{
	public:
		enum Projection
		{
			ORTHOGONAL,
			PERSPECTIVE
		};
		
		CameraActor(Projection projection = ORTHOGONAL);
		virtual ~CameraActor();
		
		virtual void SetPos(float x, float y);
		virtual void SetPos(const Vector3& pos);
		
		void SetLookAt(const Vector3& look_at, bool is_offset = false);
		
		void SetOrthoZoom(float zoom);
		void SetPerspectiveFov(float fov_y);

		void UpdateViewMatrix();
		void UpdateProjectionMatrix();
		
		void SetViewModified();
		void SetProjectionModified();
		void SetViewProjectionNeedUpdate();
		
		bool IsInFrustum(const Sphere* sphere);
		
		inline Projection projection() { return projection_; }
		inline float ortho_zoom() { return ortho_zoom_; }
		inline float perspective_fov() { return perspective_fov_y_; }
		
		inline bool is_view_need_update() { return is_view_need_update_; }
		inline bool is_projection_need_update() { return is_projection_need_update_; }
		
	private:
		void		CalculateViewMatrix();
		void		CalculateProjectionMatrix();
		
		Projection	projection_;
		
		Matrix4		view_matrix_;
		Matrix4		projection_matrix_;
		Plane		frustum_[6];
		
		Vector3		look_at_;
		bool		is_look_at_offset_;
		
		float		ortho_zoom_;
		float		perspective_fov_y_;
		
		bool		is_view_modified_, is_projection_modified_;
		bool		is_view_need_update_, is_projection_need_update_;
		bool		is_frustum_dirty_;
	};
	
#pragma mark LightActor
	
	class LightActor : public SceneActor
	{
	public:
		enum Type
		{
			POINT,
			DIRECTION,
			SPOT
		};
		
		LightActor(Type type);
		virtual ~LightActor();
		
		virtual void AddToScene(int layer_id = 0);
		virtual void RemoveFromScene();
		
		virtual void SetPos(float x, float y);
		virtual void SetPos(const Vector3& pos);

		void SetDir(const Vector3& dir);
		
		void SetAmbient(const Color& ambient);
		void SetDiffuse(const Color& diffuse);
		void SetSpecular(const Color& specular);
		
		void SetAttenuation(float constant, float linear, float quadratic);
		
		void SetSpotExponent(float exponent);
		void SetSpotCutoff(float cutoff);
		
	private:
		Type	type_;
		int		idx_;
		Color	ambient_, diffuse_, specular_;
		float	attenuation_constant_, attenuation_linear_, attenuation_quadratic_;
		
		Vector3	dir_;
		float	spot_exponent_, spot_cutoff_;
	};
	
#pragma mark SpriteActor
	
	class SpriteActor : public SceneActor
	{
	public:
		SpriteActor(float width, float height, float offset_width = 0.0f, float offset_height = 0.0f);
		virtual ~SpriteActor();
		
		void SetSizeOffset(float width, float height, float offset_width = 0.0f, float offset_height = 0.0f);
		
		void SetTexScale(float u_scale, float v_scale, bool is_tex2 = false);
		void SetTexScroll(float u_scroll, float v_scroll, bool is_tex2 = false);
		void SetTexArea(int start_x, int start_y, int width, int height, bool is_tex2 = false);
		void SetTexAreaUV(float start_u, float start_v, float width, float height, bool is_tex2 = false);
		
		void SetTxt(const std::string& txt, const std::string& font_name, float font_size);
		void SetUseLine(bool use_line);

		inline void set_is_dynamic_draw(bool is_dynamic_draw) { is_dynamic_draw_ = is_dynamic_draw; }
		inline void set_area_border(float border) { area_border_ = border; }
		
		inline const Vector2& size() { return size_; }
		inline const Vector2& offset() { return offset_; }
		
	private:
		virtual bool IsInArea(const Vector3& local_space_pos);
		
		void UpdateVertexBuffer();

		Vector2		size_;
		Vector2		offset_;
		
		Vector2		tex_scale_;
		Vector2		tex_scroll_;
		Vector2		tex_scale2_;
		Vector2		tex_scroll2_;
		
		bool		is_use_tex2_;
		
		bool		is_dynamic_draw_;
		bool		is_use_line_;
		
		float		area_border_;
	};
	
#pragma mark BoxActor
	
	class BoxActor : public SceneActor
	{
	public:
		BoxActor(const Vector3& half_ext);
		virtual ~BoxActor();
		
	private:
		void UpdateVertexBuffer();
		
		Vector3		half_ext_;
	};
	
#pragma mark NumberActor
	
	class NumberActor : public SceneActor
	{
	public:
		NumberActor(float unit_width, float unit_height, bool is_force_sign = false);
		virtual ~NumberActor();
		
		void SetTexUnit(int width, int height);
		void SetTexArea(int start_x, int start_y, int width, int height);
		
		void SetSpacing(float spacing);
		
		void SetNumber(int number);
		void SetNumberFloat(float number);
		
		int number() { return number_; }
		int now_len() { return now_len_; }
	
	private:
		virtual bool IsInArea(const Vector3& local_space_pos);
		
		void UpdateVertexBuffer();
		
		vertex_2_pos_tex*	vertices_;
		int					now_len_max_;
		int					now_len_;
		
		Vector2		size_;
		Vector2		tex_unit_uv_;
		Vector2		tex_scale_;
		Vector2		tex_scroll_;
		float		spacing_;
		
		int			number_;
		float		number_f_;
		bool		is_float_;
		bool		is_force_sign_;
	};

}

#endif // ERI_SCENE_ACTOR_H
