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

#include "renderer.h"

#if ERI_PLATFORM == ERI_PLATFORM_WIN
#include "GL/glew.h"
//#include "GL/wglew.h"
//#include "GLee.h"
#elif ERI_PLATFORM == ERI_PLATFORM_MAC
#include <OpenGL/gl.h>
#elif ERI_PLATFORM == ERI_PLATFORM_ANDROID
#include <GLES/gl.h>
#elif ERI_PLATFORM == ERI_PLATFORM_IOS
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#endif

#include "material_data.h"

#define MAX_FRAMEBUFFER	8
#define MAX_LIGHT		8

namespace ERI {
	
	class RenderContext;
	struct TextureUnit;
	
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
		
		virtual void BackingLayer(void* layer);
		virtual void Resize(int width, int height);
		
		virtual int width() { return width_; }
		virtual int height() { return height_; }

		virtual void RenderStart();
		virtual void RenderEnd();
		virtual void Render(const RenderData* data);
		virtual void ClearDepth();
		
		virtual void SaveTransform();
		virtual void RecoverTransform();
		
		virtual void EnableRenderToBuffer(int width, int height, int frame_buffer);
		virtual void CopyTexture(unsigned int texture);
		virtual void CopyPixels(void* buffer, int x, int y, int width, int height);
		virtual void RestoreRenderToBuffer();
		
		virtual void EnableBlend(bool enable);
		virtual void EnableAlphaTest(bool enable);
		virtual void EnableMaterial(const MaterialData* data);
		
		virtual void EnableLight(bool enable);
		virtual void EnableDepthTest(bool enable);
		virtual void EnableDepthWrite(bool enable);
		
		virtual void EnableTextureUnit(int idx, const TextureUnit& unit);
		virtual void DisableTextureUnit(int idx);
		
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

		virtual unsigned int GenerateTexture(const void* buffer, int width, int height, PixelFormat format);
		virtual unsigned int GenerateRenderToTexture(int width, int height, int& out_frame_buffer);
		virtual void UpdateTexture(unsigned int texture_id, const void* buffer, int width, int height, PixelFormat format);
		virtual void ReleaseTexture(int texture_id);
		virtual void ReleaseRenderToTexture(int texture_id, int frame_buffer);
		
		virtual void SetBgColor(const Color& color);
		
		virtual void UpdateView(const Vector3& eye, const Vector3& at, const Vector3& up);
		
		virtual void UpdateOrthoProjection(float width, float height, float near_z, float far_z);
		virtual void UpdateOrthoProjection(float zoom, float near_z, float far_z);
		virtual void UpdatePerspectiveProjection(float fov_y, float aspect, float near_z, float far_z);
		virtual void UpdatePerspectiveProjection(float fov_y, float near_z, float far_z);
		
		virtual void SetViewOrientation(ViewOrientation orientaion);
		
	private:
		int GenerateFrameBuffer();
		void ReleaseFrameBuffer(int frame_buffer);

		void UpdateLightTransform();
		void AdjustProjectionForViewOrientation();
		
		RenderContext*	context_;
		
		// The pixel dimensions of the CAEAGLLayer
		// TODO: bad naming
		GLint backing_width_, backing_height_;
		int backing_width_backup_, backing_height_backup_;
		int width_, height_;
		
		GLuint current_frame_buffer_;
		GLuint color_render_buffer_;
		
		GLuint frame_buffers_[MAX_FRAMEBUFFER];
		
		GLuint depth_buffer_;
		bool use_depth_buffer_;
		
		GLbitfield clear_bits_;
		
		bool vertex_normal_enable_;
		bool vertex_color_enable_;
		bool light_enable_;
		bool depth_test_enable_;
		bool depth_write_enable_;
		bool blend_enable_;
		
		bool texture_enable_;
		bool texture_unit_enable_[MAX_TEXTURE_UNIT];
		
		unsigned int now_texture_;
		
		Color bg_color_;
		Color now_color_;
		
		GLenum blend_src_factor_, blend_dst_factor_;
		
		LightInfo light_infos_[MAX_LIGHT];
	};

}

#endif // ERI_RENDERER_ES1_H
