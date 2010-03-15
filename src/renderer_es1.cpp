/*
 *  renderer_es1.cpp
 *  eri
 *
 *  Created by exe on 11/28/09.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#include "pch.h"

#include "renderer_es1.h"

#ifndef OS_ANDROID
#include "render_context_iphone.h"
#endif

#include "render_data.h"
#include "material_data.h"

#include <cstdio>
#include <cmath>
#include <cstring>

namespace ERI {
	
	RendererES1::RendererES1() :
		context_(NULL),
		width_(0),
		height_(0),
		default_frame_buffer_(0),
		color_render_buffer_(0),
		depth_buffer_(0),
		use_depth_buffer_(true),
		depth_test_enable_(true),
		depth_write_enable_(true),
		blend_enable_(false),
		texture_enable_(false),
		now_texture_(0),
		bg_color_(Color(0.0f, 0.0f, 0.0f, 0.0f))
	{
	}
	
	RendererES1::~RendererES1()
	{
#ifndef OS_ANDROID
		if (depth_buffer_)
		{
			glDeleteRenderbuffersOES(1, &depth_buffer_);
		}
		
		if (color_render_buffer_)
		{
			glDeleteRenderbuffersOES(1, &color_render_buffer_);
		}
		
		if (default_frame_buffer_)
		{
			glDeleteFramebuffersOES(1, &default_frame_buffer_);
		}
		
		if (context_) delete context_;
#endif
	}

	bool RendererES1::Init(bool use_depth_buffer)
	{
		use_depth_buffer_ = use_depth_buffer;

#ifndef OS_ANDROID
		context_ = new RenderContextIphone;
		if (!context_->Init(1))
		{
			delete context_;
			context_ = NULL;
			return false;
		}
#endif
		
		return true;
	}
	
	void RendererES1::BackingLayer(void* layer)
	{
#ifndef OS_ANDROID
		// Create default framebuffer object. The backing will be allocated for the current layer in -resizeFromLayer
		glGenFramebuffersOES(1, &default_frame_buffer_);
		glGenRenderbuffersOES(1, &color_render_buffer_);
		glBindFramebufferOES(GL_FRAMEBUFFER_OES, default_frame_buffer_);
		glBindRenderbufferOES(GL_RENDERBUFFER_OES, color_render_buffer_);
		glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, color_render_buffer_);
		
		context_->BackingLayer(layer);
		
		int backing_width, backing_height;
		
		glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &backing_width);
		glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &backing_height);
		
		if (glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES)
		{
			printf("Failed to make complete framebuffer object %x", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
			ASSERT(0);
		}
		
		if (use_depth_buffer_)
		{
			glGenRenderbuffersOES(1, &depth_buffer_);
			glBindRenderbufferOES(GL_RENDERBUFFER_OES, depth_buffer_);
			glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, backing_width, backing_height);
			glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, depth_buffer_);
			
			if (glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES)
			{
				printf("Failed to make complete framebuffer object %x", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
				ASSERT(0);
			}
		}
		
		Resize(backing_width, backing_height);
#endif
	}
	
	void RendererES1::Resize(int w, int h)
	{
		backing_width_ = w;
		backing_height_ = h;
		
		clear_bits_ = GL_COLOR_BUFFER_BIT;
		
		if (use_depth_buffer_)
		{
			clear_bits_ |= GL_DEPTH_BUFFER_BIT;
		}
		else
		{
			depth_test_enable_ = false;
		}
		
		glViewport(0, 0, backing_width_, backing_height_);
		glScissor(0, 0, backing_width_, backing_height_);
		
		SetViewOrientation(view_orientation_);
		UpdateOrthoProj(0.0f, 0.0f, 1.0f);
		
		if (depth_test_enable_)
			glEnable(GL_DEPTH_TEST);
		
		glEnable(GL_SCISSOR_TEST);
		glEnable(GL_CULL_FACE);
		
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // because pre-multiplied alpha?
		
		glEnableClientState(GL_VERTEX_ARRAY);
	}

	void RendererES1::RenderStart()
	{
#ifndef OS_ANDROID
		// This application only creates a single context which is already set current at this point.
		// This call is redundant, but needed if dealing with multiple contexts.
		context_->SetAsCurrent();
		
		// This application only creates a single default framebuffer which is already bound at this point.
		// This call is redundant, but needed if dealing with multiple framebuffers.
		glBindFramebufferOES(GL_FRAMEBUFFER_OES, default_frame_buffer_);
#endif
		
		glClearColor(bg_color_.r, bg_color_.g, bg_color_.b, bg_color_.a);
		glClear(clear_bits_);
	}
	
	void RendererES1::RenderEnd()
	{
#ifndef OS_ANDROID
		// This application only creates a single color renderbuffer which is already bound at this point.
		// This call is redundant, but needed if dealing with multiple renderbuffers.
		glBindRenderbufferOES(GL_RENDERBUFFER_OES, color_render_buffer_);
		
		context_->Present();
#endif
	}
	
	void RendererES1::Render(const RenderData* data)
	{
		glMultMatrixf(data->world_model_matrix.m);
		
		//glTranslatef(data->translate.x, data->translate.y, data->translate.z);
		//glRotatef(data->rotate_degree, data->rotate_axis.x, data->rotate_axis.y, data->rotate_axis.z);
		//glScalef(data->scale.x, data->scale.y, data->scale.z);
		
		if (data->vertex_count > 0)
		{
			if (now_color_ != data->color)
			{
				glColor4f(data->color.r, data->color.g, data->color.b, data->color.a);
				now_color_ = data->color;
			}
			
			glBindBuffer(GL_ARRAY_BUFFER, data->vertex_buffer);
			glVertexPointer(2, GL_FLOAT, sizeof(vertex_2_pos_tex), (void*)offsetof(vertex_2_pos_tex, position));

			if (texture_enable_)
			{
//				if (data->is_tex_transform)
//				{
//					glMatrixMode(GL_TEXTURE);
//					glTranslatef(data->tex_translate.x, data->tex_translate.y, 0.0f);
//					glScalef(data->tex_scale.x, data->tex_scale.y, 1.0f);
//				}
				
				glTexCoordPointer(2, GL_FLOAT, sizeof(vertex_2_pos_tex), (void*)offsetof(vertex_2_pos_tex, tex_coord));
			}
			
			glDrawArrays(data->vertex_type, 0,  data->vertex_count);
			
//			if (data->is_tex_transform)
//			{
//				glLoadIdentity();
//				glMatrixMode(GL_MODELVIEW);
//			}
		}
	}
	
	void RendererES1::Render()
	{
		// Replace the implementation of this method to do your own custom drawing
		
		static const GLfloat squareVertices[] = {
			-50.0f,  -50.0f,
			50.0f,  -50.0f,
			-50.0f,   50.0f,
			50.0f,   50.0f,
		};
		
		static const GLubyte squareColors[] = {
			255, 255,   0, 255,
			0,   255, 255, 255,
			0,     0,   0,   0,
			255,   0, 255, 255,
		};
		
		static float transY = 0.0f;

#ifndef OS_ANDROID
		// This application only creates a single context which is already set current at this point.
		// This call is redundant, but needed if dealing with multiple contexts.
		context_->SetAsCurrent();

		// This application only creates a single default framebuffer which is already bound at this point.
		// This call is redundant, but needed if dealing with multiple framebuffers.
		glBindFramebufferOES(GL_FRAMEBUFFER_OES, default_frame_buffer_);
#endif
		
		glViewport(0, 0, backing_width_, backing_height_);
		
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrthof(-backing_width_ * 0.5, backing_width_ * 0.5, -backing_height_ * 0.5, backing_height_ * 0.5, -1, 1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslatef(0.0f, (GLfloat)(sinf(transY) * 50), 0.0f);
		transY += 0.075f;
		
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		glVertexPointer(2, GL_FLOAT, 0, squareVertices);
		glEnableClientState(GL_VERTEX_ARRAY);
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, squareColors);
		glEnableClientState(GL_COLOR_ARRAY);
		
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		
#ifndef OS_ANDROID
		// This application only creates a single color renderbuffer which is already bound at this point.
		// This call is redundant, but needed if dealing with multiple renderbuffers.
		glBindRenderbufferOES(GL_RENDERBUFFER_OES, color_render_buffer_);
		
		context_->Present();
#endif
	}
	
	void RendererES1::SaveTransform()
	{
		glPushMatrix();
	}
	
	void RendererES1::RecoverTransform()
	{
		glPopMatrix();
	}

	void RendererES1::EnableDepthTest(bool enable)
	{
		if (use_depth_buffer_ && depth_test_enable_ != enable)
		{
			depth_test_enable_ = enable;
			
			if (enable)
				glEnable(GL_DEPTH_TEST);
			else
				glDisable(GL_DEPTH_TEST);
		}
	}
	
	void RendererES1::EnableDepthWrite(bool enable)
	{
		if (use_depth_buffer_ && depth_write_enable_ != enable)
		{
			depth_write_enable_ = enable;
			
			if (enable)
				glEnable(GL_DEPTH_TEST);
			else
				glDisable(GL_DEPTH_TEST);
		}
	}
	
	void RendererES1::EnableBlend(bool enable)
	{
		if (blend_enable_ != enable)
		{
			blend_enable_ = enable;
			
			if (enable)
				glEnable(GL_BLEND);
			else
				glDisable(GL_BLEND);
		}
	}
	
	void RendererES1::EnableAlphaTest(bool enable)
	{
	}
	
	void RendererES1::EnableTexture(bool enable, unsigned int texture /*= 0*/)
	{
		if (texture_enable_ != enable)
		{
			texture_enable_ = enable;
			
			if (enable)
			{
				glEnable(GL_TEXTURE_2D);
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			}
			else
			{
				glDisable(GL_TEXTURE_2D);
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			}
		}
		
		if (texture_enable_ && texture != 0 && now_texture_ != texture)
		{
			glBindTexture(GL_TEXTURE_2D, texture);
			now_texture_ = texture;
		}
	}
	
	void RendererES1::EnableMaterial(const MaterialData* data)
	{
		EnableDepthTest(data->depth_test);
		EnableDepthWrite(data->depth_write);
		
		if (data->texture)
		{
			EnableTexture(true, data->texture->id);
			if (data->texture->filter_min != data->tex_filter_min)
			{
				data->texture->filter_min = data->tex_filter_min;
				
				switch (data->texture->filter_min)
				{
					case FILTER_NEAREST:
						glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
						break;
					case FILTER_LINEAR:
						glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
						break;
					default:
						ASSERT2(0, "invalid filter type");
						break;
				}
			}
			
			if (data->texture->filter_mag != data->tex_filter_mag)
			{
				data->texture->filter_mag = data->tex_filter_mag;
				
				switch (data->texture->filter_mag)
				{
					case FILTER_NEAREST:
						glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
						break;
					case FILTER_LINEAR:
						glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
						break;
					default:
						ASSERT2(0, "invalid filter type");
						break;
				}
			}
		}
		else
		{
			EnableTexture(false);
		}
	}
	
	unsigned int RendererES1::GenerateTexture(void* buffer, int width, int height, PixelFormat format)
	{
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		now_texture_ = texture;
		
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		
		switch (format)
		{
			case RGBA:
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
				break;
			case RGB:
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, buffer);
				break;
			case ALPHA:
				glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, buffer);
				break;
			default:
				ASSERT2(0, "invalid pixel format!");
				break;
		}
		
		return texture;
	}
	
	unsigned int RendererES1::BindTexture()
	{
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		now_texture_ = texture;
		
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		
		return texture;
	}
	
	void RendererES1::UpdateOrthoProj(float center_x, float center_y, float zoom)
	{
		ASSERT(zoom);
		
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		
		switch (view_orientation_)
		{
			case PORTRAIT_HOME_BOTTOM:
				glOrthof(-backing_width_ * 0.5f / zoom + center_x, backing_width_ * 0.5f / zoom + center_x, -backing_height_ * 0.5f / zoom + center_y, backing_height_ * 0.5f / zoom + center_y, -1000, 1000);
				break;
			case PORTRAIT_HOME_TOP:
				glOrthof(-backing_width_ * 0.5f / zoom - center_x, backing_width_ * 0.5f / zoom - center_x, -backing_height_ * 0.5f / zoom - center_y, backing_height_ * 0.5f / zoom - center_y, -1000, 1000);
				glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
				break;
			case LANDSCAPE_HOME_RIGHT:
				glOrthof(-backing_width_ * 0.5f / zoom + center_y, backing_width_ * 0.5f / zoom + center_y, -backing_height_ * 0.5f / zoom - center_x, backing_height_ * 0.5f / zoom - center_x, -1000, 1000);
				glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);
				break;
			case LANDSCAPE_HOME_LEFT:
				glOrthof(-backing_width_ * 0.5f / zoom - center_y, backing_width_ * 0.5f / zoom - center_y, -backing_height_ * 0.5f / zoom + center_x, backing_height_ * 0.5f / zoom + center_x, -1000, 1000);
				glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
				break;
			default:
				ASSERT(0);
				break;
		}
		
		glMatrixMode(GL_MODELVIEW);
	}
	
	void RendererES1::UpdateViewMatrix(const Matrix4& view_matrix)
	{
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glMultMatrixf(view_matrix.m);
	}
	
	void RendererES1::SetViewOrientation(ViewOrientation orientaion)
	{
		view_orientation_ = orientaion;
		
		switch (view_orientation_)
		{
			case PORTRAIT_HOME_BOTTOM:
			case PORTRAIT_HOME_TOP:
				width_ = backing_width_;
				height_ = backing_height_;
				break;
			case LANDSCAPE_HOME_RIGHT:
			case LANDSCAPE_HOME_LEFT:
				width_ = backing_height_;
				height_ = backing_width_;
				break;
			default:
				ASSERT(0);
				break;
		}
	}
	
}
