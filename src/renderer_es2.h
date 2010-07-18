/*
 *  renderer_es2.h
 *  eri
 *
 *  Created by exe on 11/28/09.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#ifndef ERI_RENDERER_ES2_H
#define ERI_RENDERER_ES2_H

#include "renderer.h"

#include <string>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

namespace ERI {
	
	class RenderContext;
	
	class RendererES2 : public Renderer
	{
	public:
		RendererES2();
		virtual ~RendererES2();
		
		virtual bool Init(bool use_depth_buffer);
		virtual void BackingLayer(void* layer);
		virtual void Resize(int width, int height) {}
		
		virtual int width() { return backing_width_; }
		virtual int height() { return backing_height_; }
		
		virtual void RenderStart();
		virtual void RenderEnd();
		virtual void Render(const RenderData* data);
		virtual void Render();
		
		virtual void SaveTransform() {}
		virtual void RecoverTransform() {}
		
		virtual void EnableRenderToBuffer(int width, int height, int frame_buffer) {}
		virtual void RestoreRenderToBuffer() {}
		
		virtual void ReleaseFrameBuffer(int frame_buffer) {}
		
		virtual void EnableBlend(bool enable) {}
		virtual void EnableAlphaTest(bool enable) {}
		virtual void EnableMaterial(const MaterialData* data) {}
		
		virtual void ObtainLight(int& idx) {}
		virtual void ReleaseLight(int idx) {}
		virtual void SetLightPos(int idx, const Vector3& pos) {}
		virtual void SetLightDir(int idx, const Vector3& dir) {}
		virtual void SetLightSpotDir(int idx, const Vector3& dir) {}
		virtual void SetLightAmbient(int idx, const Color& ambient) {}
		virtual void SetLightDiffuse(int idx, const Color& diffuse) {}
		virtual void SetLightSpecular(int idx, const Color& specular) {}
		virtual void SetLightAttenuation(int idx, float constant, float linear, float quadratic) {}
		virtual void SetLightSpotExponent(int idx, float exponent) {}
		virtual void SetLightSpotCutoff(int idx, float cutoff) {}
		
		virtual unsigned int GenerateTexture(void* buffer, int width, int height, PixelFormat format) { return 0; }
		virtual unsigned int GenerateRenderToTexture(int width, int height, int& out_frame_buffer) { return 0; }
		virtual void ReleaseTexture(int texture_id) {}
		
		virtual void SetBgColor(const Color& color) {}
		
		virtual void UpdateView(const Vector3& eye, const Vector3& at, const Vector3& up) {}
		
		virtual void UpdateOrthoProjection(float width, float height, float near, float far) {}
		virtual void UpdateOrthoProjection(float zoom, float near, float far) {}
		virtual void UpdatePerspectiveProjection(float fov_y, float aspect, float near, float far) {}
		virtual void UpdatePerspectiveProjection(float fov_y, float near, float far) {}
		
		virtual void SetViewOrientation(ViewOrientation orientaion) { view_orientation_ = orientaion; }
	
	private:
		bool LoadShaders();
		bool CompileShader(GLuint* shader, GLenum type, const std::string& file_path);
		bool LinkProgram(GLuint program);
		bool ValidateProgram(GLuint program);
		
		RenderContext*	context_;
		
		// The pixel dimensions of the CAEAGLLayer
		GLint backing_width_;
		GLint backing_height_;
		
		// The OpenGL names for the framebuffer and renderbuffer used to render to this view
		GLuint default_frame_buffer_;
		GLuint color_render_buffer_;
		
		GLuint program_;
	};
	
}

#endif // ERI_RENDERER_ES2_H