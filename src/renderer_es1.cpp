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

#include <cmath>

#if ERI_PLATFORM == ERI_PLATFORM_WIN
#include "win/render_context_win.h"
#elif ERI_PLATFORM == ERI_PLATFORM_IOS
#include "ios/render_context_ios.h"
#endif

#include "root.h"
#include "scene_mgr.h"
#include "render_data.h"
#include "material_data.h"

namespace ERI {
	
	const int kDefaultFrameBufferIdx = 0;
	
	RendererES1::RendererES1() :
		context_(NULL),
		width_(0),
		height_(0),
		current_frame_buffer_(0),
		color_render_buffer_(0),
		depth_buffer_(0),
		use_depth_buffer_(true),
		vertex_normal_enable_(false),
		light_enable_(false),
		depth_test_enable_(true),
		depth_write_enable_(true),
		blend_enable_(false),
		texture_enable_(false),
		now_texture_(0),
		bg_color_(Color(0.0f, 0.0f, 0.0f, 0.0f)),
		blend_src_factor_(GL_SRC_ALPHA),
		blend_dst_factor_(GL_ONE_MINUS_SRC_ALPHA)
	{
		memset(frame_buffers_, 0, sizeof(frame_buffers_));
	}
	
	RendererES1::~RendererES1()
	{
#if ERI_PLATFORM == ERI_PLATFORM_IOS
		if (depth_buffer_)
		{
			glDeleteRenderbuffersOES(1, &depth_buffer_);
		}
		
		if (color_render_buffer_)
		{
			glDeleteRenderbuffersOES(1, &color_render_buffer_);
		}
		
		for (int i = 0; i < MAX_FRAMEBUFFER; ++i)
		{
			if (frame_buffers_[i])
			{
				glDeleteFramebuffersOES(1, &frame_buffers_[i]);
			}
		}
#endif

		if (context_) delete context_;
	}

	bool RendererES1::Init(bool use_depth_buffer)
	{
		use_depth_buffer_ = use_depth_buffer;

#if ERI_PLATFORM == ERI_PLATFORM_WIN
		context_ = new RenderContextWin;
#elif ERI_PLATFORM == ERI_PLATFORM_IOS
		context_ = new RenderContextIphone;
#endif

		if (context_ && !context_->Init(1))
		{
			delete context_;
			context_ = NULL;
			return false;
		}
		
		clear_bits_ = GL_COLOR_BUFFER_BIT;
		
		if (use_depth_buffer_)
		{
			clear_bits_ |= GL_DEPTH_BUFFER_BIT;
		}
		else
		{
			depth_test_enable_ = false;
		}
		
		if (depth_test_enable_)
		{
			glEnable(GL_DEPTH_TEST);
		}
		
		glEnable(GL_SCISSOR_TEST);
		glEnable(GL_CULL_FACE);
		glBlendFunc(blend_src_factor_, blend_dst_factor_);
		//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // because pre-multiplied alpha?
		glEnable(GL_COLOR_MATERIAL);
		glEnableClientState(GL_VERTEX_ARRAY);

		SetBgColor(bg_color_);
		
		return true;
	}
	
	void RendererES1::BackingLayer(void* layer)
	{
#if ERI_PLATFORM == ERI_PLATFORM_IOS

		ASSERT(context_);

		// Create default framebuffer object. The backing will be allocated for the current layer in -resizeFromLayer
		glGenFramebuffersOES(1, &frame_buffers_[kDefaultFrameBufferIdx]);
		glGenRenderbuffersOES(1, &color_render_buffer_);
		glBindFramebufferOES(GL_FRAMEBUFFER_OES, frame_buffers_[kDefaultFrameBufferIdx]);
		glBindRenderbufferOES(GL_RENDERBUFFER_OES, color_render_buffer_);
		glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, color_render_buffer_);
		
		context_->BackingLayer(layer);
		
		int backing_width, backing_height;
		
		glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &backing_width);
		glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &backing_height);
		
		GLenum status = glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES);
		if (status != GL_FRAMEBUFFER_COMPLETE_OES)
		{
			printf("Failed to make complete framebuffer object %x", status);
			ASSERT(0);
		}
		
		if (use_depth_buffer_)
		{
			glGenRenderbuffersOES(1, &depth_buffer_);
			glBindRenderbufferOES(GL_RENDERBUFFER_OES, depth_buffer_);
			glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, backing_width, backing_height);
			glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, depth_buffer_);
			
			status = glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES);
			if (status != GL_FRAMEBUFFER_COMPLETE_OES)
			{
				printf("Failed to make complete framebuffer object %x", status);
				ASSERT(0);
			}
		}
		
		current_frame_buffer_ = frame_buffers_[kDefaultFrameBufferIdx];
		
		Resize(backing_width, backing_height);
		
		// clear initial display
		glClear(clear_bits_);
		if (context_) context_->Present();
#endif
	}
	
	void RendererES1::Resize(int width, int height)
	{
		backing_width_ = width;
		backing_height_ = height;
		
		SetViewOrientation(view_orientation_);
		
		glViewport(0, 0, backing_width_, backing_height_);
		glScissor(0, 0, backing_width_, backing_height_);
		
		Root::Ins().scene_mgr()->OnRenderResize();
	}

	void RendererES1::RenderStart()
	{
		// This application only creates a single context which is already set current at this point.
		// This call is redundant, but needed if dealing with multiple contexts.
		//if (context_) context_->SetAsCurrent();

#if ERI_PLATFORM == ERI_PLATFORM_IOS
		// This application only creates a single default framebuffer which is already bound at this point.
		// This call is redundant, but needed if dealing with multiple framebuffers.
		glBindFramebufferOES(GL_FRAMEBUFFER_OES, current_frame_buffer_);
#endif
		
		glClear(clear_bits_);
	}
	
	void RendererES1::RenderEnd()
	{
#if ERI_PLATFORM == ERI_PLATFORM_IOS
		// This application only creates a single color renderbuffer which is already bound at this point.
		// This call is redundant, but needed if dealing with multiple renderbuffers.
		glBindRenderbufferOES(GL_RENDERBUFFER_OES, color_render_buffer_);
#endif
		
		if (context_) context_->Present();
	}
	
	void RendererES1::Render(const RenderData* data)
	{
		glMultMatrixf(data->world_model_matrix.m);
		
		if (data->vertex_count > 0)
		{
			if (now_color_ != data->color)
			{
				glColor4f(data->color.r, data->color.g, data->color.b, data->color.a);
				now_color_ = data->color;
			}
			
			if (blend_enable_ && (blend_src_factor_ != data->blend_src_factor || blend_dst_factor_ != data->blend_dst_factor))
			{
				blend_src_factor_ = data->blend_src_factor;
				blend_dst_factor_ = data->blend_dst_factor;
				glBlendFunc(blend_src_factor_, blend_dst_factor_);
			}
			
			glBindBuffer(GL_ARRAY_BUFFER, data->vertex_buffer);
			
			switch (data->vertex_format)
			{
				case POS_TEX_2:
					glVertexPointer(2, GL_FLOAT, sizeof(vertex_2_pos_tex), (void*)offsetof(vertex_2_pos_tex, position));
					if (vertex_normal_enable_)
					{
						vertex_normal_enable_ = false;
						glDisableClientState(GL_NORMAL_ARRAY);
					}
					if (texture_enable_)
					{
						//if (data->is_tex_transform)
						//{
						//	glMatrixMode(GL_TEXTURE);
						//	glTranslatef(data->tex_translate.x, data->tex_translate.y, 0.0f);
						//	glScalef(data->tex_scale.x, data->tex_scale.y, 1.0f);
						//}
						
						glTexCoordPointer(2, GL_FLOAT, sizeof(vertex_2_pos_tex), (void*)offsetof(vertex_2_pos_tex, tex_coord));
					}
					break;
					
				case POS_NORMAL_TEX_3:
					glVertexPointer(3, GL_FLOAT, sizeof(vertex_3_pos_normal_tex), (void*)offsetof(vertex_3_pos_normal_tex, position));
					if (!vertex_normal_enable_)
					{
						vertex_normal_enable_ = true;
						glEnableClientState(GL_NORMAL_ARRAY);
					}
					glNormalPointer(GL_FLOAT, sizeof(vertex_3_pos_normal_tex), (void*)offsetof(vertex_3_pos_normal_tex, normal));
					if (texture_enable_)
					{
						glTexCoordPointer(2, GL_FLOAT, sizeof(vertex_3_pos_normal_tex), (void*)offsetof(vertex_3_pos_normal_tex, tex_coord));
					}
					break;

				default:
					break;
			}
			
			if (data->index_count > 0)
			{
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->index_buffer);
				glDrawElements(data->vertex_type, data->index_count, GL_UNSIGNED_SHORT, 0);
			}
			else
			{
				glDrawArrays(data->vertex_type, 0,  data->vertex_count);
			}
			
//			if (data->is_tex_transform)
//			{
//				glLoadIdentity();
//				glMatrixMode(GL_MODELVIEW);
//			}
		}
	}
	
	void RendererES1::SaveTransform()
	{
		glPushMatrix();
	}
	
	void RendererES1::RecoverTransform()
	{
		glPopMatrix();
	}
	
	void RendererES1::EnableRenderToBuffer(int width, int height, int frame_buffer)
	{
		backing_width_backup_ = backing_width_;
		backing_height_backup_ = backing_height_;
		backing_width_ = width;
		backing_height_ = height;
		
		glViewport(0, 0, backing_width_, backing_height_);

#if ERI_PLATFORM == ERI_PLATFORM_IOS
		current_frame_buffer_ = frame_buffer;
#endif
	}

	void RendererES1::CopyTexture(unsigned int texture)
	{
#if ERI_PLATFORM == ERI_PLATFORM_WIN
		glBindTexture(GL_TEXTURE_2D, texture);
		now_texture_ = texture;

		// Copy Our ViewPort To The Blur Texture (From 0,0 To 128,128... No Border)
		glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, backing_width_, backing_height_, 0);
#endif
	}
	
	void RendererES1::RestoreRenderToBuffer()
	{
		backing_width_ = backing_width_backup_;
		backing_height_ = backing_height_backup_;
		
		glViewport(0, 0, backing_width_, backing_height_);

#if ERI_PLATFORM == ERI_PLATFORM_IOS
		current_frame_buffer_ = frame_buffers_[kDefaultFrameBufferIdx];
#endif
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
	
	void RendererES1::EnableMaterial(const MaterialData* data)
	{
		EnableLight(data->accept_light);
		EnableDepthTest(data->depth_test);
		EnableDepthWrite(data->depth_write);
		
		if (data->texture)
		{
			EnableTexture(true, data->texture->id);
			if (data->texture->current_params.filter_min != data->custom_params.filter_min)
			{
				data->texture->current_params.filter_min = data->custom_params.filter_min;
				
				switch (data->texture->current_params.filter_min)
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
			
			if (data->texture->current_params.filter_mag != data->custom_params.filter_mag)
			{
				data->texture->current_params.filter_mag = data->custom_params.filter_mag;
				
				switch (data->texture->current_params.filter_mag)
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
	
	void RendererES1::EnableLight(bool enable)
	{
		if (light_enable_ != enable)
		{
			light_enable_ = enable;
			
			if (enable)
				glEnable(GL_LIGHTING);
			else
				glDisable(GL_LIGHTING);
		}
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
	
	void RendererES1::ObtainLight(int& idx)
	{
		for (int i = 0; i < MAX_LIGHT; ++i)
		{
			if (!light_infos_[i].used)
			{
				light_infos_[i].used = true;
				idx = i;
				
				glEnable(GL_LIGHT0 + idx);
				
				return;
			}
		}
	}
	
	void RendererES1::ReleaseLight(int idx)
	{
		ASSERT(idx >= 0 && idx < MAX_LIGHT);
		ASSERT(light_infos_[idx].used);
		
		light_infos_[idx].Clear();
		
		glDisable(GL_LIGHT0 + idx);
		// clear setting?
	}
	
	void RendererES1::SetLightPos(int idx, const Vector3& pos)
	{
		ASSERT(idx >= 0 && idx < MAX_LIGHT);
		ASSERT(light_infos_[idx].used);
		
		light_infos_[idx].pos = pos;
		light_infos_[idx].pos_w = 1.0f;
		
		GLfloat params[] = { pos.x, pos.y, pos.z, 1.0f };
		glLightfv(GL_LIGHT0 + idx, GL_POSITION, params);
	}
	
	void RendererES1::SetLightDir(int idx, const Vector3& dir)
	{
		ASSERT(idx >= 0 && idx < MAX_LIGHT);
		ASSERT(light_infos_[idx].used);
		
		light_infos_[idx].pos = dir;
		light_infos_[idx].pos_w = 0.0f;
		
		GLfloat params[] = { dir.x, dir.y, dir.z, 0.0f };
		glLightfv(GL_LIGHT0 + idx, GL_POSITION, params);
	}
	
	void RendererES1::SetLightSpotDir(int idx, const Vector3& dir)
	{
		ASSERT(idx >= 0 && idx < MAX_LIGHT);
		ASSERT(light_infos_[idx].used);
		
		light_infos_[idx].spot_dir = dir;
		
		GLfloat params[] = { dir.x, dir.y, dir.z };
		glLightfv(GL_LIGHT0 + idx, GL_SPOT_DIRECTION, params);
	}
	
	void RendererES1::SetLightAmbient(int idx, const Color& ambient)
	{
		ASSERT(idx >= 0 && idx < MAX_LIGHT);
		ASSERT(light_infos_[idx].used);
		
		GLfloat params[] = { ambient.r, ambient.g, ambient.b, 1.0f };
		glLightfv(GL_LIGHT0 + idx, GL_AMBIENT, params);
	}
	
	void RendererES1::SetLightDiffuse(int idx, const Color& diffuse)
	{
		ASSERT(idx >= 0 && idx < MAX_LIGHT);
		ASSERT(light_infos_[idx].used);
		
		GLfloat params[] = { diffuse.r, diffuse.g, diffuse.b, 1.0f };
		glLightfv(GL_LIGHT0 + idx, GL_DIFFUSE, params);
	}
	
	void RendererES1::SetLightSpecular(int idx, const Color& specular)
	{
		ASSERT(idx >= 0 && idx < MAX_LIGHT);
		ASSERT(light_infos_[idx].used);
		
		GLfloat params[] = { specular.r, specular.g, specular.b, 1.0f };
		glLightfv(GL_LIGHT0 + idx, GL_SPECULAR, params);
	}
	
	void RendererES1::SetLightAttenuation(int idx, float constant, float linear, float quadratic)
	{
		ASSERT(idx >= 0 && idx < MAX_LIGHT);
		ASSERT(light_infos_[idx].used);

		glLightf(GL_LIGHT0 + idx, GL_CONSTANT_ATTENUATION, constant);
		glLightf(GL_LIGHT0 + idx, GL_LINEAR_ATTENUATION, linear);
		glLightf(GL_LIGHT0 + idx, GL_QUADRATIC_ATTENUATION, quadratic);
	}
	
	void RendererES1::SetLightSpotExponent(int idx, float exponent)
	{
		ASSERT(idx >= 0 && idx < MAX_LIGHT);
		ASSERT(light_infos_[idx].used);
		
		glLightf(GL_LIGHT0 + idx, GL_SPOT_EXPONENT, exponent);
	}
	
	void RendererES1::SetLightSpotCutoff(int idx, float cutoff)
	{
		ASSERT(idx >= 0 && idx < MAX_LIGHT);
		ASSERT(light_infos_[idx].used);
		
		glLightf(GL_LIGHT0 + idx, GL_SPOT_CUTOFF, cutoff);
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
		//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		
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
	
	unsigned int RendererES1::GenerateRenderToTexture(int width, int height, int& out_frame_buffer)
	{
		// create the framebuffer object
		int frame_buffer = GenerateFrameBuffer();

#if ERI_PLATFORM != ERI_PLATFORM_WIN
		if (!frame_buffer)
			return 0;
#endif

#if ERI_PLATFORM == ERI_PLATFORM_IOS
		glBindFramebufferOES(GL_FRAMEBUFFER_OES, frame_buffer);
#endif
		
		// create the texture
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,  width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		now_texture_ = texture;
		
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// attach the texture to the framebuffer
#if ERI_PLATFORM == ERI_PLATFORM_IOS
		glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, texture, 0);
		
		// TODO: use depth buffer?

		//// allocate and attach a depth buffer
		//GLuint depth_render_buffer;
		//glGenRenderbuffersOES(1, &depth_render_buffer);
		//glBindRenderbufferOES(GL_RENDERBUFFER_OES, depth_render_buffer);
		//glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, width, height);
		//glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, depth_render_buffer);

		GLenum status = glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) ;
		if(status != GL_FRAMEBUFFER_COMPLETE_OES)
		{
			printf("Failed to make complete framebuffer object %x", status);
			ASSERT(0);
		}
#endif
		
		out_frame_buffer = frame_buffer;
		
		return texture;
	}
	
	void RendererES1::ReleaseTexture(int texture_id)
	{
		ASSERT(texture_id > 0);
		
		if (now_texture_ == texture_id)
		{
			glBindTexture(GL_TEXTURE_2D, 0);
			now_texture_ = 0;
		}

		GLuint id = texture_id;
		glDeleteTextures(1, &id);
	}

	void RendererES1::ReleaseRenderToTexture(int texture_id, int frame_buffer)
	{
		ReleaseTexture(texture_id);
		ReleaseFrameBuffer(frame_buffer);
	}

	void RendererES1::SetBgColor(const Color& color)
	{
		bg_color_ = color;
		glClearColor(bg_color_.r, bg_color_.g, bg_color_.b, bg_color_.a);
	}
	
	void RendererES1::UpdateView(const Vector3& eye, const Vector3& at, const Vector3& up)
	{
		static Matrix4 view;
		MatrixLookAtRH(view, eye, at, up);
		
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(view.m);
		
		UpdateLightTransform();
	}
	
	void RendererES1::UpdateOrthoProjection(float width, float height, float near_z, float far_z)
	{
		GLint original_matrix_mode;
		glGetIntegerv(GL_MATRIX_MODE, &original_matrix_mode);
		
		if (original_matrix_mode != GL_PROJECTION)
		{
			glPushMatrix();
			glMatrixMode(GL_PROJECTION);
		}
		
		/*
		static Matrix4 projection;
		MatrixOrthoRH(projection, width, height, near_z, far_z);
		glLoadMatrixf(projection.m);
		 */
		
		glLoadIdentity();

#if ERI_PLATFORM == ERI_PLATFORM_WIN
		glOrtho(-width * 0.5f, width * 0.5f, -height * 0.5f, height * 0.5f, near_z, far_z);
#else
		glOrthof(-width * 0.5f, width * 0.5f, -height * 0.5f, height * 0.5f, near_z, far_z);
#endif
		
		AdjustProjectionForViewOrientation();
		
		if (original_matrix_mode != GL_PROJECTION)
		{
			glMatrixMode(original_matrix_mode);
			glPopMatrix();
		}
	}
	
	void RendererES1::UpdateOrthoProjection(float zoom, float near_z, float far_z)
	{
		ASSERT(zoom);
		
		UpdateOrthoProjection(backing_width_ / zoom, backing_height_ / zoom, near_z, far_z);
	}
	
	void RendererES1::UpdatePerspectiveProjection(float fov_y, float aspect, float near_z, float far_z)
	{
		GLint original_matrix_mode;
		glGetIntegerv(GL_MATRIX_MODE, &original_matrix_mode);
		
		if (original_matrix_mode != GL_PROJECTION)
		{
			glPushMatrix();
			glMatrixMode(GL_PROJECTION);
		}

		/*
		static Matrix4 projection;
		MatrixPerspectiveFovRH(projection, fov_y, aspect, near_z, far_z);
		glLoadMatrixf(projection.m);
		 */
		
		glLoadIdentity();
		 
		float angle = fov_y * 0.5f;
		float half_height = near_z * tan(angle);
		float half_width = aspect * half_height;
		
#if ERI_PLATFORM == ERI_PLATFORM_WIN
		glFrustum(-half_width, half_width, -half_height, half_height, near_z, far_z);
#else
		glFrustumf(-half_width, half_width, -half_height, half_height, near_z, far_z);
#endif
		
		AdjustProjectionForViewOrientation();
		
		if (original_matrix_mode != GL_PROJECTION)
		{
			glMatrixMode(original_matrix_mode);
			glPopMatrix();
		}
	}

	void RendererES1::UpdatePerspectiveProjection(float fov_y, float near_z, float far_z)
	{
		UpdatePerspectiveProjection(fov_y, static_cast<float>(backing_width_) / backing_height_, near_z, far_z);
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
	
	int RendererES1::GenerateFrameBuffer()
	{
#if ERI_PLATFORM == ERI_PLATFORM_IOS
		for (int i = kDefaultFrameBufferIdx + 1; i < MAX_FRAMEBUFFER; ++i)
		{
			if (!frame_buffers_[i])
			{
				glGenFramebuffersOES(1, &frame_buffers_[i]);
				return frame_buffers_[i];
			}
		}
#endif
		
		return 0;
	}

	void RendererES1::ReleaseFrameBuffer(int frame_buffer)
	{
#if ERI_PLATFORM == ERI_PLATFORM_IOS
		ASSERT(frame_buffer > 0);

		for (int i = 0; i < MAX_FRAMEBUFFER; ++i)
		{
			if (frame_buffers_[i] == frame_buffer)
			{
				glDeleteFramebuffersOES(1, &frame_buffers_[i]);
				frame_buffers_[i] = 0;
				return;
			}
		}
#endif
	}
	
	void RendererES1::UpdateLightTransform()
	{
		for (int i = 0; i < MAX_LIGHT; ++i)
		{
			if (light_infos_[i].used)
			{
				GLfloat params1[] = { light_infos_[i].pos.x, light_infos_[i].pos.y, light_infos_[i].pos.z, light_infos_[i].pos_w };
				glLightfv(GL_LIGHT0 + i, GL_POSITION, params1);
				
				GLfloat params2[] = { light_infos_[i].spot_dir.x, light_infos_[i].spot_dir.y, light_infos_[i].spot_dir.z };
				glLightfv(GL_LIGHT0 + i, GL_SPOT_DIRECTION, params2);
			}
		}
	}
	
	void RendererES1::AdjustProjectionForViewOrientation()
	{
		switch (view_orientation_)
		{
			case PORTRAIT_HOME_BOTTOM:
				break;
			case PORTRAIT_HOME_TOP:
				glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
				break;
			case LANDSCAPE_HOME_RIGHT:
				glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);
				break;
			case LANDSCAPE_HOME_LEFT:
				glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
				break;
			default:
				ASSERT(0);
				break;
		}
	}
	
}
