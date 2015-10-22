/*
 *  renderer.h
 *  eri
 *
 *  Created by exe on 11/28/09.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#ifndef ERI_RENDERER_H
#define ERI_RENDERER_H

#include <string>

#include "math_helper.h"

namespace ERI {
	
	enum ViewOrientation
	{
		PORTRAIT_HOME_BOTTOM,
		PORTRAIT_HOME_TOP,
		LANDSCAPE_HOME_RIGHT,
		LANDSCAPE_HOME_LEFT
	};
	
	enum PixelFormat
	{
		ALPHA,
		RGB,
		RGBA,
		RGBA_PVR_4BPP,
		RGBA_PVR_2BPP
	};
	
	enum FogMode
	{
		FOG_LINEAR,
		FOG_EXP,
		FOG_EXP2
	};
	
	struct RenderData;
	struct MaterialData;
	
	struct Caps
	{
		Caps()
			: max_texture_size(0),
			is_support_non_power_of_2_texture(false)
		{
		}
		
		std::string version;
		
		int		max_texture_size;
		bool	is_support_non_power_of_2_texture;
	};
	
	class Renderer
	{
	public:
		Renderer()
			: view_orientation_(PORTRAIT_HOME_BOTTOM),
			content_scale_(1.0f) {}
		
		virtual ~Renderer() {}
		
		virtual bool Init(bool use_depth_buffer) = 0;
		
		virtual void SetContextAsCurrent() = 0;
		
		// TODO: bad naming
		virtual void BackingLayer(const void* layer) = 0;
		virtual void Resize(int width, int height) = 0;
		
		virtual int width() = 0;
		virtual int height() = 0;
		virtual int backing_width() = 0;
		virtual int backing_height() = 0;
		
		virtual bool IsReadyToRender() = 0;
		virtual void RenderStart() = 0;
		virtual void RenderEnd() = 0;
		virtual void Render(const RenderData* data) = 0;
		virtual void ClearDepth() = 0;
		
		virtual void SaveTransform() = 0;
		virtual void RecoverTransform() = 0;
		
		virtual void EnableRenderToBuffer(int x, int y, int width, int height, int frame_buffer) = 0;
		virtual void CopyTexture(unsigned int texture, PixelFormat format) = 0;
		virtual void CopyPixels(void* buffer, int x, int y, int width, int height, PixelFormat format) = 0;
		virtual void RestoreRenderToBuffer() = 0;
		
		virtual void EnableBlend(bool enable) = 0;
		virtual void EnableAlphaTest(bool enable) = 0;
		virtual void EnableMaterial(const MaterialData* data) = 0;
		
		virtual void ObtainLight(int& idx) = 0;
		virtual void ReleaseLight(int idx) = 0;
		virtual void SetLightPos(int idx, const Vector3& pos) = 0;
		virtual void SetLightDir(int idx, const Vector3& dir) = 0;
		virtual void SetLightSpotDir(int idx, const Vector3& dir) = 0;
		virtual void SetLightAmbient(int idx, const Color& ambient) = 0;
		virtual void SetLightDiffuse(int idx, const Color& diffuse) = 0;
		virtual void SetLightSpecular(int idx, const Color& specular) = 0;
		virtual void SetLightAttenuation(int idx, float constant, float linear, float quadratic) = 0;
		virtual void SetLightSpotExponent(int idx, float exponent) = 0;
		virtual void SetLightSpotCutoff(int idx, float cutoff) = 0;

		virtual void SetFog(FogMode mode, float density = 1.f) = 0;
		virtual void SetFogDistance(float start, float end = 1.f) = 0;
		virtual void SetFogColor(const Color& color) = 0;

		virtual unsigned int GenerateTexture(const void* buffer, int width, int height, PixelFormat format, int buffer_size = 0) = 0;
		virtual unsigned int GenerateTexture() = 0;
		virtual void UpdateTexture(unsigned int texture_id, const void* buffer, int width, int height, PixelFormat format) = 0;
		virtual void ReleaseTexture(int texture_id) = 0;
		
		virtual void BindDefaultFrameBuffer() = 0;
		virtual int GenerateFrameBuffer() = 0;
		virtual void BindTextureToFrameBuffer(unsigned int texture_id, int frame_buffer) = 0;
		virtual void ReleaseFrameBuffer(int frame_buffer) = 0;
		
		virtual void ReleaseRenderData(RenderData& data) = 0;
		
		virtual void SetBgColor(const Color& color) = 0;
		virtual const Color& GetBgColor() = 0;
		
		virtual void SetClearDepth(float clamped_depth) = 0;
		
		virtual void UpdateView(const Matrix4& view_matrix) = 0;
		virtual void UpdateView(const Vector3& eye, const Vector3& at, const Vector3& up) = 0;

		virtual void UpdateProjection(const Matrix4& projection_matrix) = 0;
		virtual void UpdateOrthoProjection(float width, float height, float near, float far) = 0;
		virtual void UpdateOrthoProjection(float zoom, float near, float far) = 0;
		virtual void UpdatePerspectiveProjection(float fov_y, float aspect, float near, float far) = 0;
		virtual void UpdatePerspectiveProjection(float fov_y, float near, float far) = 0;
		
		virtual void SetViewOrientation(ViewOrientation orientaion) = 0;
		inline ViewOrientation view_orientation() { return view_orientation_; }
		
		inline void set_content_scale(float scale) { content_scale_ = scale; }
		inline float content_scale() { return content_scale_; }
		
		inline const Caps& caps() { return caps_; }
		
	protected:
		ViewOrientation	view_orientation_;
		Caps			caps_;
		
	private:
		float			content_scale_;
	};
}

#endif // ERI_RENDERER_H
