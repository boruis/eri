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
#else
#include "render_context.h"
#endif

#include "root.h"
#include "scene_mgr.h"
#include "render_data.h"
#include "material_data.h"

namespace ERI {
	
	const int kDefaultFrameBufferIdx = 0;
	
	const GLint kParamFilters[] =
	{
		GL_NEAREST,
		GL_LINEAR,
		GL_NEAREST_MIPMAP_NEAREST,
		GL_LINEAR_MIPMAP_NEAREST,
		GL_NEAREST_MIPMAP_LINEAR,
		GL_LINEAR_MIPMAP_LINEAR
	};
	
	const GLint kParamWraps[] =
	{
		GL_REPEAT,
		GL_CLAMP_TO_EDGE
	};
	
	const GLint kEnvModes[] =
	{
		GL_REPLACE,
		GL_MODULATE,
		GL_DECAL,
		GL_BLEND,
		GL_ADD
	};
	
	const GLint kEnvOps[] =
	{
		GL_REPLACE,
		GL_MODULATE,
		GL_ADD,
		GL_ADD_SIGNED,
		GL_INTERPOLATE,
		GL_SUBTRACT,
		GL_DOT3_RGB,
		GL_DOT3_RGBA
	};
	
	const GLint kEnvSrcs[] =
	{
		GL_TEXTURE,
		//GL_TEXTUREn, ?
		GL_CONSTANT,
		GL_PRIMARY_COLOR,
		GL_PREVIOUS
	};
	
	const GLint kEnvOperands[] =
	{
		GL_SRC_COLOR,
		GL_ONE_MINUS_SRC_COLOR,
		GL_SRC_ALPHA,
		GL_ONE_MINUS_SRC_ALPHA
	};
	
	RendererES1::RendererES1() :
		context_(NULL),
		width_(0),
		height_(0),
		current_frame_buffer_(0),
		color_render_buffer_(0),
		depth_buffer_(0),
		use_depth_buffer_(true),
		vertex_normal_enable_(false),
		vertex_color_enable_(false),
		light_enable_(false),
		depth_test_enable_(true),
		depth_write_enable_(true),
		blend_enable_(false),
		alpha_test_enable_(false),
		texture_enable_(false),
		now_texture_(0),
		bg_color_(Color(0.0f, 0.0f, 0.0f, 0.0f)),
		blend_src_factor_(GL_SRC_ALPHA),
		blend_dst_factor_(GL_ONE_MINUS_SRC_ALPHA),
		alpha_test_func_(GL_GREATER),
		alpha_test_ref_(0.0f)
	{
		memset(frame_buffers_, 0, sizeof(frame_buffers_));
		memset(texture_unit_enable_, 0, sizeof(texture_unit_enable_));
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
		context_->Present();

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
		if (context_) context_->SetAsCurrent();

#if ERI_PLATFORM == ERI_PLATFORM_IOS
		// This application only creates a single default framebuffer which is already bound at this point.
		// This call is redundant, but needed if dealing with multiple framebuffers.
		glBindFramebufferOES(GL_FRAMEBUFFER_OES, current_frame_buffer_);
#endif
	
		EnableDepthWrite(true);
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
	
	void RendererES1::ClearDepth()
	{
		if (use_depth_buffer_)
		{
			EnableDepthWrite(true);
			glClear(GL_DEPTH_BUFFER_BIT);
		}
	}
	
	void RendererES1::Render(const RenderData* data)
	{
		if (data->apply_identity_model_matrix)
			glLoadMatrixf(current_view_matrix_.m);
		else
			glMultMatrixf(data->world_model_matrix.m);
		
		if (data->vertex_count > 0)
		{
			if (now_color_ != data->color)
			{
				glColor4f(data->color.r, data->color.g, data->color.b, data->color.a);
				now_color_ = data->color;
			}
			
			if (blend_enable_ &&
				(blend_src_factor_ != data->blend_src_factor ||
				 blend_dst_factor_ != data->blend_dst_factor))
			{
				blend_src_factor_ = data->blend_src_factor;
				blend_dst_factor_ = data->blend_dst_factor;
				glBlendFunc(blend_src_factor_, blend_dst_factor_);
			}
			
			if (alpha_test_enable_ &&
				(alpha_test_func_ != data->alpha_test_func ||
				 alpha_test_ref_ != data->alpha_test_ref))
			{
				alpha_test_func_ = data->alpha_test_func;
				alpha_test_ref_ = data->alpha_test_ref;
				glAlphaFunc(alpha_test_func_, alpha_test_ref_);
			}
			
			glBindBuffer(GL_ARRAY_BUFFER, data->vertex_buffer);
			
			GLint vertex_pos_size, vertex_stride;
			void* vertex_pos_offset;
			void* vertex_normal_offset;
			void* vertex_tex_coord_offset[MAX_TEXTURE_UNIT];
			void* vertex_color_offset;
			bool use_vertex_normal = false;
			bool use_vertex_color = false;

			switch (data->vertex_format)
			{
				case POS_TEX_2:
					vertex_pos_size = 2;
					vertex_stride = sizeof(vertex_2_pos_tex);
					vertex_pos_offset = (void*)offsetof(vertex_2_pos_tex, position);
					vertex_tex_coord_offset[0] = (void*)offsetof(vertex_2_pos_tex, tex_coord);
					for (int i = 1; i < MAX_TEXTURE_UNIT; ++i) {
						vertex_tex_coord_offset[i] = vertex_tex_coord_offset[0];
					}
					break;
					
				case POS_TEX2_2:
					vertex_pos_size = 2;
					vertex_stride = sizeof(vertex_2_pos_tex2);
					vertex_pos_offset = (void*)offsetof(vertex_2_pos_tex2, position);
					vertex_tex_coord_offset[0] = (void*)offsetof(vertex_2_pos_tex2, tex_coord);
					vertex_tex_coord_offset[1] = (void*)offsetof(vertex_2_pos_tex2, tex_coord2);
					for (int i = 2; i < MAX_TEXTURE_UNIT; ++i) {
						vertex_tex_coord_offset[i] = vertex_tex_coord_offset[0];
					}
					break;
					
				case POS_TEX_COLOR_2:
					vertex_pos_size = 2;
					vertex_stride = sizeof(vertex_2_pos_tex_color);
					vertex_pos_offset = (void*)offsetof(vertex_2_pos_tex_color, position);
					vertex_tex_coord_offset[0] = (void*)offsetof(vertex_2_pos_tex_color, tex_coord);
					for (int i = 1; i < MAX_TEXTURE_UNIT; ++i) {
						vertex_tex_coord_offset[i] = vertex_tex_coord_offset[0];
					}
					vertex_color_offset = (void*)offsetof(vertex_2_pos_tex_color, color);
					use_vertex_color = true;
					break;
					
				case POS_NORMAL_3:
					vertex_pos_size = 3;
					vertex_stride = sizeof(vertex_3_pos_normal);
					vertex_pos_offset = (void*)offsetof(vertex_3_pos_normal, position);
					vertex_normal_offset = (void*)offsetof(vertex_3_pos_normal, normal);
					use_vertex_normal = true;
					break;
					
				case POS_NORMAL_TEX_3:
					vertex_pos_size = 3;
					vertex_stride = sizeof(vertex_3_pos_normal_tex);
					vertex_pos_offset = (void*)offsetof(vertex_3_pos_normal_tex, position);
					vertex_normal_offset = (void*)offsetof(vertex_3_pos_normal_tex, normal);
					vertex_tex_coord_offset[0] = (void*)offsetof(vertex_3_pos_normal_tex, tex_coord);
					for (int i = 1; i < MAX_TEXTURE_UNIT; ++i) {
						vertex_tex_coord_offset[i] = vertex_tex_coord_offset[0];
					}
					use_vertex_normal = true;
					break;
					
				case POS_NORMAL_COLOR_TEX_3:
					vertex_pos_size = 3;
					vertex_stride = sizeof(vertex_3_pos_normal_color_tex);
					vertex_pos_offset = (void*)offsetof(vertex_3_pos_normal_color_tex, position);
					vertex_normal_offset = (void*)offsetof(vertex_3_pos_normal_color_tex, normal);
					vertex_tex_coord_offset[0] = (void*)offsetof(vertex_3_pos_normal_color_tex, tex_coord);
					for (int i = 1; i < MAX_TEXTURE_UNIT; ++i) {
						vertex_tex_coord_offset[i] = vertex_tex_coord_offset[0];
					}
					use_vertex_normal = true;
					vertex_color_offset = (void*)offsetof(vertex_3_pos_normal_color_tex, color);
					use_vertex_color = true;
					break;
					
				case POS_TEX_COLOR_3:
					vertex_pos_size = 3;
					vertex_stride = sizeof(vertex_3_pos_tex_color);
					vertex_pos_offset = (void*)offsetof(vertex_3_pos_tex_color, position);
					vertex_tex_coord_offset[0] = (void*)offsetof(vertex_3_pos_tex_color, tex_coord);
					for (int i = 1; i < MAX_TEXTURE_UNIT; ++i) {
						vertex_tex_coord_offset[i] = vertex_tex_coord_offset[0];
					}
					vertex_color_offset = (void*)offsetof(vertex_3_pos_tex_color, color);
					use_vertex_color = true;
					break;

				default:
					ASSERT(0);
					break;
			}
		
			// pos
			glVertexPointer(vertex_pos_size, GL_FLOAT, vertex_stride, vertex_pos_offset);
			// normal
			if (vertex_normal_enable_ != use_vertex_normal)
			{
				vertex_normal_enable_ = use_vertex_normal;
				if (vertex_normal_enable_)
					glEnableClientState(GL_NORMAL_ARRAY);
				else
					glDisableClientState(GL_NORMAL_ARRAY);
			}
			if (vertex_normal_enable_)
			{
				glNormalPointer(GL_FLOAT, vertex_stride, vertex_normal_offset);
			}
			// color
			if (vertex_color_enable_ != use_vertex_color)
			{
				vertex_color_enable_ = use_vertex_color;
				if (vertex_color_enable_)
					glEnableClientState(GL_COLOR_ARRAY);
				else
					glDisableClientState(GL_COLOR_ARRAY);
			}
			if (vertex_color_enable_)
			{
				glColorPointer(4, GL_FLOAT, vertex_stride, vertex_color_offset);
			}
			// tex coord
			if (texture_enable_)
			{
				if (data->is_tex_transform)
				{
					SaveTransform();
					
					glMatrixMode(GL_TEXTURE);
					glLoadIdentity();
					glTranslatef(data->tex_translate.x, data->tex_translate.y, 0.0f);
					glScalef(data->tex_scale.x, data->tex_scale.y, 1.0f);
				}
				
				for (int i = 0; i < MAX_TEXTURE_UNIT; ++i)
				{
					if (texture_unit_enable_[i])
					{
						glClientActiveTexture(GL_TEXTURE0 + i);
						glTexCoordPointer(2, GL_FLOAT, vertex_stride, vertex_tex_coord_offset[i]);
					}
				}
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
			
			if (data->is_tex_transform)
			{
				glLoadIdentity();
				glMatrixMode(GL_MODELVIEW);
				
				RecoverTransform();
			}
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
#if ERI_PLATFORM == ERI_PLATFORM_WIN || ERI_PLATFORM == ERI_PLATFORM_MAC
		glBindTexture(GL_TEXTURE_2D, texture);
		now_texture_ = texture;

		// Copy Our ViewPort To The Blur Texture (From 0,0 To 128,128... No Border)
		glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, backing_width_, backing_height_, 0);
#endif
	}
	
	void RendererES1::CopyPixels(void* buffer, int x, int y, int width, int height)
	{
		glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
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
		if (alpha_test_enable_ != enable)
		{
			alpha_test_enable_ = enable;
			
			if (enable)
				glEnable(GL_ALPHA_TEST);
			else
				glDisable(GL_ALPHA_TEST);
		}
	}
	
	void RendererES1::EnableMaterial(const MaterialData* data)
	{
		EnableLight(data->accept_light);
		EnableDepthTest(data->depth_test);
		EnableDepthWrite(data->depth_write);
		
		texture_enable_ = (data->used_unit > 0);
		
		for (int i = 0; i < MAX_TEXTURE_UNIT; ++i)
		{
			if (i < data->used_unit)
				EnableTextureUnit(i, data->texture_units[i]);
			else
				DisableTextureUnit(i);
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
				glDepthMask(GL_TRUE);
			else
				glDepthMask(GL_FALSE);
		}
	}
	
	void RendererES1::EnableTextureUnit(int idx, const TextureUnit& unit)
	{
		GLenum tex_enum = GL_TEXTURE0 + idx;
		
		glActiveTexture(tex_enum);
		
		if (unit.texture)
		{
			ASSERT(unit.texture->id);
			
			if (now_texture_ != unit.texture->id)
			{
				now_texture_ = unit.texture->id;
				glBindTexture(GL_TEXTURE_2D, now_texture_);
			}
			
			if (unit.texture->current_params.filter_min != unit.params.filter_min)
			{
				unit.texture->current_params.filter_min = unit.params.filter_min;
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, kParamFilters[unit.params.filter_min]);
			}
			if (unit.texture->current_params.filter_mag != unit.params.filter_mag)
			{
				unit.texture->current_params.filter_mag = unit.params.filter_mag;
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, kParamFilters[unit.params.filter_mag]);
			}
			if (unit.texture->current_params.wrap_s != unit.params.wrap_s)
			{
				unit.texture->current_params.wrap_s = unit.params.wrap_s;
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, kParamWraps[unit.params.wrap_s]);
			}
			if (unit.texture->current_params.wrap_t != unit.params.wrap_t)
			{
				unit.texture->current_params.wrap_t = unit.params.wrap_t;
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, kParamWraps[unit.params.wrap_t]);
			}
		}
			
		if (unit.envs.mode == MODE_COMBINE)
		{
			if (unit.envs.is_use_constant_color)
			{
				GLfloat color[4] = { unit.envs.constant_color.r, unit.envs.constant_color.g, unit.envs.constant_color.b, unit.envs.constant_color.a };
				glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color);
			}
			
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, kEnvOps[unit.envs.combine_rgb]);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, kEnvSrcs[unit.envs.src0_rgb]);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, kEnvSrcs[unit.envs.src1_rgb]);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_RGB, kEnvSrcs[unit.envs.src2_rgb]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, kEnvOperands[unit.envs.operand0_rgb]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, kEnvOperands[unit.envs.operand1_rgb]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, kEnvOperands[unit.envs.operand2_rgb]);
			
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, kEnvOps[unit.envs.combine_alpha]);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, kEnvSrcs[unit.envs.src0_alpha]);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, kEnvSrcs[unit.envs.src1_alpha]);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_ALPHA, kEnvSrcs[unit.envs.src2_alpha]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, kEnvOperands[unit.envs.operand0_alpha]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, kEnvOperands[unit.envs.operand1_alpha]);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, kEnvOperands[unit.envs.operand2_alpha]);
		}
		else
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, kEnvModes[unit.envs.mode]);
		}

		if (!texture_unit_enable_[idx])
		{
			glEnable(GL_TEXTURE_2D);
			
			glClientActiveTexture(GL_TEXTURE0 + idx);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		
		texture_unit_enable_[idx] = true;
	}
	
	void RendererES1::DisableTextureUnit(int idx)
	{
		if (texture_unit_enable_[idx])
		{
			glActiveTexture(GL_TEXTURE0 + idx);
			glDisable(GL_TEXTURE_2D);
			
			glClientActiveTexture(GL_TEXTURE0 + idx);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		
		texture_unit_enable_[idx] = false;
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
	
	unsigned int RendererES1::GenerateTexture(const void* buffer, int width, int height, PixelFormat format)
	{
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		now_texture_ = texture;
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		
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
		
		//glGenerateMipmap(GL_TEXTURE_2D);
		
		return texture;
	}
	
	unsigned int RendererES1::GenerateRenderToTexture(int width, int height, int& out_frame_buffer)
	{
		// create the framebuffer object
		int frame_buffer = GenerateFrameBuffer();

#if ERI_PLATFORM != ERI_PLATFORM_WIN && ERI_PLATFORM != ERI_PLATFORM_MAC
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
		now_texture_ = texture;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,  width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		
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
	
	void RendererES1::UpdateTexture(unsigned int texture_id, const void* buffer, int width, int height, PixelFormat format)
	{
		ASSERT(texture_id > 0);

		glBindTexture(GL_TEXTURE_2D, texture_id);
		now_texture_ = texture_id;
		
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
		MatrixLookAtRH(current_view_matrix_, eye, at, up);
		
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(current_view_matrix_.m);
		
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

#if ERI_PLATFORM == ERI_PLATFORM_WIN || ERI_PLATFORM == ERI_PLATFORM_MAC
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
		
#if ERI_PLATFORM == ERI_PLATFORM_WIN || ERI_PLATFORM == ERI_PLATFORM_MAC
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
