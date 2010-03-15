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
#include "texture_mgr.h"
#include "math_helper.h"

#ifdef OS_ANDROID
#include <GLES/gl.h>
#else
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#endif

namespace ERI {
	
	class RenderContext;

	class RendererES1 : public Renderer
	{
	public:
		RendererES1();
		virtual ~RendererES1();
		
		virtual bool Init(bool use_depth_buffer);
		
		virtual void BackingLayer(void* layer);
		virtual void Resize(int w, int h);
		
		virtual int width() { return width_; }
		virtual int height() { return height_; }

		virtual void RenderStart();
		virtual void RenderEnd();
		virtual void Render(const RenderData* data);
		virtual void Render();
		
		virtual void SaveTransform();
		virtual void RecoverTransform();
		
		virtual void EnableDepthTest(bool enable);
		virtual void EnableDepthWrite(bool enable);
		virtual void EnableBlend(bool enable);
		virtual void EnableAlphaTest(bool enable);
		virtual void EnableTexture(bool enable, unsigned int texture = 0);
		virtual void EnableMaterial(const MaterialData* data);
		
		virtual unsigned int GenerateTexture(void* buffer, int width, int height, PixelFormat format);
		virtual unsigned int BindTexture();
		
		virtual void SetBgColor(const Color& color) { bg_color_ = color; }
		
		virtual void UpdateOrthoProj(float center_x, float center_y, float zoom);
		virtual void UpdateViewMatrix(const Matrix4& view_matrix);
		
		virtual void SetViewOrientation(ViewOrientation orientaion);
		
	private:
		RenderContext*	context_;
		
		// The pixel dimensions of the CAEAGLLayer
		GLint backing_width_, backing_height_;
		int width_, height_;
		
		// The OpenGL names for the framebuffer and renderbuffer used to render to this view
		GLuint default_frame_buffer_;
		GLuint color_render_buffer_;
		
		GLuint depth_buffer_;
		bool use_depth_buffer_;
		
		GLbitfield clear_bits_;

		bool depth_test_enable_;
		bool depth_write_enable_;
		bool blend_enable_;
		bool texture_enable_;
		unsigned int now_texture_;
		
		Color bg_color_;
		Color now_color_;
	};

}

#endif // ERI_RENDERER_ES1_H
