/*
 *  renderer_es1.h
 *  eri
 *
 *  Created by exe on 11/28/09.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#ifndef ERI_RENDERER_ES1_H
#define ERI_RENDERER_ES1_H

#include "pch.h"

#ifdef ERI_RENDERER_ES1

#include "renderer.h"
#include "material_data.h"

#define MAX_LIGHT		8

namespace ERI {
	
	struct TextureUnit;
	class RenderContext;
	
	struct LightInfo
	{
		LightInfo() { Clear(); }
		
		void Clear()
		{
			used = false;
			pos = Vector3(0, 0, 1);
			pos_w = 0;
			spot_dir = Vector3(0, 0, -1);
		}
		
		bool	used;
		Vector3	pos;
		float	pos_w;
		Vector3	spot_dir;
	};

	class RendererES1 : public Renderer
	{
	public:
		RendererES1();
		virtual ~RendererES1();
		
		virtual bool Init(bool use_depth_buffer);
		
		virtual void SetContextAsCurrent();
		
		virtual void BackingLayer(const void* layer);
		virtual void Resize(int width, int height);
		
		virtual int width() { return width_; }
		virtual int height() { return height_; }
		virtual int backing_width() { return backing_width_; }
		virtual int backing_height() { return backing_height_; }

		virtual bool IsReadyToRender();
		virtual void RenderStart();
		virtual void RenderEnd();
		virtual void Render(const RenderData* data);
		virtual void ClearDepth();
		
		virtual void SaveTransform();
		virtual void RecoverTransform();
		
		virtual void EnableRenderToBuffer(int x, int y, int width, int height, int frame_buffer);
		virtual void CopyTexture(unsigned int texture, PixelFormat format);
		virtual void CopyPixels(void* buffer, int x, int y, int width, int height, PixelFormat format);
		virtual void RestoreRenderToBuffer();
		
		virtual void EnableBlend(bool enable);
		virtual void EnableAlphaTest(bool enable);
		virtual void EnableMaterial(const MaterialData* data);
		
		void EnableLight(bool enable);
		void EnableFog(bool enable);
		void EnableDepthTest(bool enable);
		void EnableDepthWrite(bool enable);
		void EnableCullFace(bool enable, bool cull_front);
		void EnableColorWrite(ColorFlags enable);
		
		void EnableTextureUnit(int idx, const TextureUnit& unit);
		void DisableTextureUnit(int idx);
		
		virtual void ObtainLight(int& idx);
		virtual void ReleaseLight(int idx);
		virtual void SetLightPos(int idx, const Vector3& pos);
		virtual void SetLightDir(int idx, const Vector3& dir);
		virtual void SetLightSpotDir(int idx, const Vector3& dir);
		virtual void SetLightAmbient(int idx, const Color& ambient);
		virtual void SetLightDiffuse(int idx, const Color& diffuse);
		virtual void SetLightSpecular(int idx, const Color& specular);
		virtual void SetLightAttenuation(int idx, float constant, float linear, float quadratic);
		virtual void SetLightSpotExponent(int idx, float exponent);
		virtual void SetLightSpotCutoff(int idx, float cutoff);

		virtual void SetFog(FogMode mode, float density = 1.f);
		virtual void SetFogDistance(float start, float end = 1.f);
		virtual void SetFogColor(const Color& color);

		virtual unsigned int GenerateTexture(const void* buffer, int width, int height, PixelFormat format, int buffer_size = 0);
		virtual unsigned int GenerateTexture();
		virtual void UpdateTexture(unsigned int texture_id, const void* buffer, int width, int height, PixelFormat format);
		virtual void ReleaseTexture(int texture_id);
		
		virtual void BindDefaultFrameBuffer();
		virtual int GenerateFrameBuffer();
		virtual void BindTextureToFrameBuffer(unsigned int texture_id, int frame_buffer);
		virtual void ReleaseFrameBuffer(int frame_buffer);

		virtual void ReleaseRenderData(RenderData& data);

		virtual void SetBgColor(const Color& color);
		virtual const Color& GetBgColor();
		
		virtual void SetClearDepth(float clamped_depth);
		
		virtual void UpdateView(const Matrix4& view_matrix);
		virtual void UpdateView(const Vector3& eye, const Vector3& at, const Vector3& up);
		
		virtual void UpdateProjection(const Matrix4& projection_matrix);
		virtual void UpdateOrthoProjection(float width, float height, float near_z, float far_z);
		virtual void UpdateOrthoProjection(float zoom, float near_z, float far_z);
		virtual void UpdatePerspectiveProjection(float fov_y, float aspect, float near_z, float far_z);
		virtual void UpdatePerspectiveProjection(float fov_y, float near_z, float far_z);
		
		virtual void SetViewOrientation(ViewOrientation orientaion);
		
	private:
		void ActiveTextureUnit(GLenum idx);
		void ClientActiveTextureUnit(GLenum idx);

		void UpdateLightTransform();

		void AdjustProjectionForViewOrientation();
		
		static const int kMaxFrameBuffer = 8;
		static const int kDefaultFrameBufferIdx = 0;
		
		RenderContext*	context_;
		
		// The pixel dimensions of the CAEAGLLayer
		// TODO: bad naming
		GLint backing_width_, backing_height_;
		int backing_width_backup_, backing_height_backup_;
		int width_, height_;
		
		GLuint frame_buffers_[kMaxFrameBuffer];
		GLuint color_render_buffer_;
		
		GLuint depth_buffer_;
		bool use_depth_buffer_;
		
		GLbitfield clear_bits_;
		Color bg_color_, now_color_;

		GLenum blend_src_factor_, blend_dst_factor_;
		bool blend_enable_;

		GLenum		alpha_test_func_;
		GLclampf	alpha_test_ref_;
		bool alpha_test_enable_;

		GLenum		depth_test_func_;
		bool depth_test_enable_;
		bool depth_write_enable_;

		bool cull_face_enable_;
		bool cull_front_;
		
		ColorFlags	color_write_enable_;
		
		bool texture_enable_;
		int texture_unit_coord_idx_[MAX_TEXTURE_UNIT];
		TextureEnvMode	texture_unit_env_mode_[MAX_TEXTURE_UNIT];

		GLenum now_active_texture_unit_, now_client_active_texture_unit_;
		unsigned int now_texture_;

		bool vertex_normal_enable_;
		bool vertex_color_enable_;
		
		bool light_enable_;
		bool fog_enable_;
		
		LightInfo light_infos_[MAX_LIGHT];
		
		Matrix4		current_view_matrix_;
	};

}

#endif //ERI_RENDERER_ES1

#endif // ERI_RENDERER_ES1_H
