/*
 *  renderer_es2.cpp
 *  eri
 *
 *  Created by exe on 11/28/09.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#include "renderer_es2.h"

#ifdef ERI_RENDERER_ES2

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
#include "platform_helper.h"
#include "shader_mgr.h"

namespace ERI {

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

	RendererES2::RendererES2() :
		context_(NULL),
		backing_width_(0),
		backing_height_(0),
		width_(0),
		height_(0),
		color_render_buffer_(0),
		depth_buffer_(0),
		use_depth_buffer_(true),
		bg_color_(Color(0.0f, 0.0f, 0.0f, 0.0f)),
		blend_src_factor_(GL_SRC_ALPHA),
		blend_dst_factor_(GL_ONE_MINUS_SRC_ALPHA),
		blend_enable_(false),
		alpha_test_func_(GL_GREATER),
		alpha_test_ref_(0.0f),
		alpha_test_enable_(false),
		depth_test_func_(GL_LESS),
		depth_test_enable_(true),
		depth_write_enable_(true),
		cull_face_enable_(true),
		cull_front_(false),
		texture_enable_(false),
		now_active_texture_unit_(0),
		now_texture_(0),
		is_view_proj_dirty_(true),
		fog_mode_(FOG_LINEAR),
		fog_density_(1.f),
		fog_start_(0.f),
		fog_end_(1000.f)
	{
		memset(frame_buffers_, 0, sizeof(frame_buffers_));
		
		for (int i = 0; i < MAX_TEXTURE_UNIT; ++i)
			texture_unit_coord_idx_[i] = -1;
	}
	
	RendererES2::~RendererES2()
	{
		if (context_) context_->SetAsCurrent();

#if ERI_PLATFORM == ERI_PLATFORM_IOS
		if (depth_buffer_)
		{
			glDeleteRenderbuffers(1, &depth_buffer_);
		}
		
		if (color_render_buffer_)
		{
			glDeleteRenderbuffers(1, &color_render_buffer_);
		}
		
		for (int i = 0; i < kMaxFrameBuffer; ++i)
		{
			if (frame_buffers_[i])
			{
				glDeleteFramebuffers(1, &frame_buffers_[i]);
			}
		}
#endif
		
		if (context_) delete context_;
	}
	
	bool RendererES2::Init(bool use_depth_buffer)
	{
		use_depth_buffer_ = use_depth_buffer;
		
#if ERI_PLATFORM == ERI_PLATFORM_WIN
		context_ = new RenderContextWin;
#elif ERI_PLATFORM == ERI_PLATFORM_IOS
		context_ = new RenderContextIphone;
#endif

		if (context_ && !context_->Init())
		{
			delete context_;
			context_ = NULL;
			return false;
		}

		//

		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &caps_.max_texture_size);
		
		// TODO: check
		// NOTE: npot no mipmap and only GL_CLAMP_TO_EDGE as wrap mode
		caps_.is_support_non_power_of_2_texture = true;
		
		//
		
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
		
		SetBgColor(bg_color_);

		return true;
	}

	void RendererES2::SetContextAsCurrent()
	{
		if (context_) context_->SetAsCurrent();
	}

	void RendererES2::BackingLayer(const void* layer)
	{
#if ERI_PLATFORM == ERI_PLATFORM_IOS
		ASSERT(context_);

		context_->SetAsCurrent();

		if (depth_buffer_)
			glDeleteRenderbuffers(1, &depth_buffer_);
		
		if (color_render_buffer_)
			glDeleteRenderbuffers(1, &color_render_buffer_);
		
		if (frame_buffers_[kDefaultFrameBufferIdx])
			glDeleteFramebuffers(1, &frame_buffers_[kDefaultFrameBufferIdx]);
		
		// Create default framebuffer object. The backing will be allocated for the current layer in -resizeFromLayer
		glGenFramebuffers(1, &frame_buffers_[kDefaultFrameBufferIdx]);
		glGenRenderbuffers(1, &color_render_buffer_);

		glBindFramebuffer(GL_FRAMEBUFFER, frame_buffers_[kDefaultFrameBufferIdx]);
		glBindRenderbuffer(GL_RENDERBUFFER, color_render_buffer_);

		context_->BackingLayer(layer);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color_render_buffer_);
		
		int backing_width, backing_height;
		
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &backing_width);
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &backing_height);
		
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		ASSERT2(status == GL_FRAMEBUFFER_COMPLETE, "Failed to make complete framebuffer object %x", status);
		
		if (use_depth_buffer_)
		{
			glGenRenderbuffers(1, &depth_buffer_);
			glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer_);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, backing_width, backing_height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer_);
			
			status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			ASSERT2(status == GL_FRAMEBUFFER_COMPLETE, "Failed to make complete framebuffer object %x", status);
		}
		
		Resize(backing_width, backing_height);
#endif
	}

	void RendererES2::Resize(int width, int height)
	{
		backing_width_ = width;
		backing_height_ = height;
		
		SetViewOrientation(view_orientation_);
		
		glViewport(0, 0, backing_width_, backing_height_);
		glScissor(0, 0, backing_width_, backing_height_);

		Root::Ins().scene_mgr()->OnViewportResize();
	}
  
	bool RendererES2::IsReadyToRender()
	{
#if ERI_PLATFORM == ERI_PLATFORM_IOS
		return frame_buffers_[kDefaultFrameBufferIdx];
#else
		return true;
#endif
	}
	
	void RendererES2::RenderStart()
	{
		// This application only creates a single context which is already set current at this point.
		// This call is redundant, but needed if dealing with multiple contexts.
		if (context_) context_->SetAsCurrent();
		
		EnableDepthWrite(true);
		glClear(clear_bits_);
	}

	void RendererES2::RenderEnd()
	{
#if ERI_PLATFORM == ERI_PLATFORM_IOS
		// This application only creates a single color renderbuffer which is already bound at this point.
		// This call is redundant, but needed if dealing with multiple renderbuffers.
		glBindRenderbuffer(GL_RENDERBUFFER, color_render_buffer_);
		
		// TODO: check support GL_EXT_discard_framebuffer
		if (use_depth_buffer_)
		{
			GLenum attachments[] = { GL_DEPTH_ATTACHMENT };
			glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, attachments);
		}
#endif
		
		if (context_) context_->Present();
	}
	
	void RendererES2::Render(const RenderData* data)
	{
		if (data->vertex_count <= 0)
			return;
		
		GLenum data_blend_src_factor = data->blend_src_factor;
		if (data->alpha_premultiplied && data_blend_src_factor == GL_SRC_ALPHA) // TODO: other situation?
		{
			data_blend_src_factor = GL_ONE;
		}
		
		if (blend_enable_ &&
			(blend_src_factor_ != data_blend_src_factor ||
			 blend_dst_factor_ != data->blend_dst_factor))
		{
			blend_src_factor_ = data_blend_src_factor;
			blend_dst_factor_ = data->blend_dst_factor;
			glBlendFunc(blend_src_factor_, blend_dst_factor_);
		}
		
		if (alpha_test_enable_ &&
			(alpha_test_func_ != data->alpha_test_func ||
			 alpha_test_ref_ != data->alpha_test_ref))
		{
			alpha_test_func_ = data->alpha_test_func;
			alpha_test_ref_ = data->alpha_test_ref;
			
			// TODO: handle in fragment shader
		}
		
		if (depth_test_enable_ &&
			depth_test_func_ != data->depth_test_func)
		{
			depth_test_func_ = data->depth_test_func;
			glDepthFunc(depth_test_func_);
		}
		
		//

		if (is_view_proj_dirty_)
		{
			Matrix4::Multiply(current_view_proj_matrix_, current_proj_matrix_, current_view_matrix_);
			is_view_proj_dirty_ = false;
		}
		
		//
		
		const std::vector<int>& uniforms = Root::Ins().shader_mgr()->current_program()->uniforms();
		
		if (data->apply_identity_model_matrix)
			tmp_matrix_[0] = current_view_proj_matrix_;
		else
			Matrix4::Multiply(tmp_matrix_[0], current_view_proj_matrix_, data->world_model_matrix);
		
		glUniformMatrix4fv(uniforms[UNIFORM_MODEL_VIEW_PROJ_MATRIX], 1, GL_FALSE, tmp_matrix_[0].m);
		
		//
		
		if (data->material_ref->accept_fog)
		{
			if (data->apply_identity_model_matrix)
				tmp_matrix_[0] = current_view_matrix_;
			else
				Matrix4::Multiply(tmp_matrix_[0], current_view_matrix_, data->world_model_matrix);
			
			glUniformMatrix4fv(uniforms[UNIFORM_MODEL_VIEW_MATRIX], 1, GL_FALSE, tmp_matrix_[0].m);
			
			glUniform1i(uniforms[UNIFORM_FOG_ENABLE], 1);
			glUniform1i(uniforms[UNIFORM_FOG_MODE], fog_mode_);
			glUniform4f(uniforms[UNIFORM_FOG_COLOR], fog_color_.r, fog_color_.g, fog_color_.b, fog_color_.a);

			switch (fog_mode_)
			{
				case FOG_LINEAR:
					glUniform1f(uniforms[UNIFORM_FOG_START], fog_start_);
					glUniform1f(uniforms[UNIFORM_FOG_END], fog_end_);
					break;

				case FOG_EXP:
				case FOG_EXP2:
					glUniform1f(uniforms[UNIFORM_FOG_DENSITY], fog_density_);
					break;

				default:
					ASSERT(0);
					break;
			}
		}
		else
		{
			glUniform1i(uniforms[UNIFORM_FOG_ENABLE], 0);
		}
		
		//
		
		glBindBuffer(GL_ARRAY_BUFFER, data->vertex_buffer);
		
		GLint vertex_pos_size, vertex_stride;
		void* vertex_pos_offset = NULL;
		void* vertex_normal_offset = NULL;
		void* vertex_tex_coord_offset[4];
		void* vertex_color_offset = NULL;
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
				
			case POS_TEX2_COLOR_2:
				vertex_pos_size = 2;
				vertex_stride = sizeof(vertex_2_pos_tex2_color);
				vertex_pos_offset = (void*)offsetof(vertex_2_pos_tex2_color, position);
				vertex_tex_coord_offset[0] = (void*)offsetof(vertex_2_pos_tex2_color, tex_coord);
				vertex_tex_coord_offset[1] = (void*)offsetof(vertex_2_pos_tex2_color, tex_coord2);
				for (int i = 2; i < MAX_TEXTURE_UNIT; ++i) {
					vertex_tex_coord_offset[i] = vertex_tex_coord_offset[0];
				}
				vertex_color_offset = (void*)offsetof(vertex_2_pos_tex2_color, color);
				use_vertex_color = true;
				break;
				
			case POS_NORMAL_3:
				vertex_pos_size = 3;
				vertex_stride = sizeof(vertex_3_pos_normal);
				vertex_pos_offset = (void*)offsetof(vertex_3_pos_normal, position);
				vertex_normal_offset = (void*)offsetof(vertex_3_pos_normal, normal);
				use_vertex_normal = true;
				ASSERT(!texture_enable_);
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
				
			case POS_COLOR_TEX_3:
				vertex_pos_size = 3;
				vertex_stride = sizeof(vertex_3_pos_color_tex);
				vertex_pos_offset = (void*)offsetof(vertex_3_pos_color_tex, position);
				vertex_tex_coord_offset[0] = (void*)offsetof(vertex_3_pos_color_tex, tex_coord);
				for (int i = 1; i < MAX_TEXTURE_UNIT; ++i) {
					vertex_tex_coord_offset[i] = vertex_tex_coord_offset[0];
				}
				vertex_color_offset = (void*)offsetof(vertex_3_pos_color_tex, color);
				use_vertex_color = true;
				break;
				
			default:
				ASSERT(0);
				break;
		}
		
		glVertexAttribPointer(ATTRIB_VERTEX, vertex_pos_size, GL_FLOAT, GL_FALSE, vertex_stride, vertex_pos_offset);
		glEnableVertexAttribArray(ATTRIB_VERTEX);

		if (use_vertex_normal)
		{
			glVertexAttribPointer(ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, vertex_stride, vertex_normal_offset);
			glEnableVertexAttribArray(ATTRIB_NORMAL);
		}
		else
		{
			glDisableVertexAttribArray(ATTRIB_NORMAL);
		}
		
		if (use_vertex_color)
		{
			glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, 1, vertex_stride, vertex_color_offset);
			glEnableVertexAttribArray(ATTRIB_COLOR);
		}
		else
		{
			GLfloat color[4] = { data->color.r, data->color.g, data->color.b, data->color.a };
			glVertexAttrib4fv(ATTRIB_COLOR, color);
			glDisableVertexAttribArray(ATTRIB_COLOR);
		}
		
		if (texture_enable_)
		{
			// TODO: more than 2 texture unit usage

			GLint tex_enable[2] = { texture_unit_coord_idx_[0] >= 0, texture_unit_coord_idx_[1] >= 0 };
			glUniform1iv(uniforms[UNIFORM_TEX_ENABLE], 2, tex_enable);
			
			if (tex_enable[0] || tex_enable[1])
			{
				if (data->is_tex_transform)
				{
					GLint tex_mat_enable[2] = { 1, 1 };
					glUniform1iv(uniforms[UNIFORM_TEX_MATRIX_ENABLE], 2, tex_mat_enable);
					
					Matrix4::Translate(tmp_matrix_[0], Vector3(data->tex_translate.x, data->tex_translate.y, 0.0f));
					Matrix4::Scale(tmp_matrix_[1], Vector3(data->tex_scale.x, data->tex_scale.y, 1.0f));
					Matrix4::Multiply(tmp_matrix_[2], tmp_matrix_[1], tmp_matrix_[0]);
					
					for (int i = 0; i < 2; ++i)
					{
						if (tex_enable[i])
							glUniformMatrix4fv(uniforms[UNIFORM_TEX_MATRIX0 + i], 1, GL_FALSE, tmp_matrix_[2].m);
					}
				}
				else
				{
					GLint tex_mat_enable[2] = { 0, 0 };
					glUniform1iv(uniforms[UNIFORM_TEX_MATRIX_ENABLE], 2, tex_mat_enable);
				}
			}
						
			for (int i = 0; i < 2; ++i)
			{
				if (tex_enable[i])
				{
					glVertexAttribPointer(ATTRIB_TEXCOORD0 + i, 2, GL_FLOAT, GL_FALSE, vertex_stride, vertex_tex_coord_offset[texture_unit_coord_idx_[i]]);
					glEnableVertexAttribArray(ATTRIB_TEXCOORD0 + i);
				}
				else
				{
					glDisableVertexAttribArray(ATTRIB_TEXCOORD0 + i);
				}
			}
		}
		else
		{
			GLint tex_enable[2] = { 0, 0 };
			glUniform1iv(uniforms[UNIFORM_TEX_ENABLE], 2, tex_enable);

			glDisableVertexAttribArray(ATTRIB_TEXCOORD0);
			glDisableVertexAttribArray(ATTRIB_TEXCOORD1);
		}
		
#if defined(DEBUG)
		if (!Root::Ins().shader_mgr()->current_program()->Validate())
			return;
#endif
		
		if (data->index_count > 0)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->index_buffer);
			glDrawElements(data->vertex_type, data->index_count, GL_UNSIGNED_SHORT, 0);
		}
		else
		{
			glDrawArrays(data->vertex_type, 0,  data->vertex_count);
		}
	}

	void RendererES2::ClearDepth()
	{
		if (use_depth_buffer_)
		{
			EnableDepthWrite(true);
			glClear(GL_DEPTH_BUFFER_BIT);
		}
	}
	
	void RendererES2::EnableRenderToBuffer(int x, int y, int width, int height, int frame_buffer)
	{
//		backing_width_backup_ = backing_width_;
//		backing_height_backup_ = backing_height_;
//		backing_width_ = width;
//		backing_height_ = height;
//		
//		glViewport(x, y, backing_width_, backing_height_);
		
		glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	}
	
	void RendererES2::CopyTexture(unsigned int texture, PixelFormat format)
	{
		glBindTexture(GL_TEXTURE_2D, texture);
		now_texture_ = texture;
		
		switch (format)
		{
			case RGBA:
				glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, backing_width_, backing_height_, 0);
				break;
			case RGB:
				glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, backing_width_, backing_height_, 0);
				break;
			case ALPHA:
				glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 0, 0, backing_width_, backing_height_, 0);
				break;
			default:
				ASSERT2(0, "invalid pixel format!");
				break;
		}
	}
	
	void RendererES2::CopyPixels(void* buffer, int x, int y, int width, int height, PixelFormat format)
	{
		switch (format)
		{
			case RGBA:
				glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
				break;
			case RGB:
				glReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, buffer);
				break;
			case ALPHA:
				glReadPixels(x, y, width, height, GL_ALPHA, GL_UNSIGNED_BYTE, buffer);
				break;
			default:
				ASSERT2(0, "invalid pixel format!");
				break;
		}
	}
	
	void RendererES2::RestoreRenderToBuffer()
	{
//		backing_width_ = backing_width_backup_;
//		backing_height_ = backing_height_backup_;
//		
//		glViewport(0, 0, backing_width_, backing_height_);
		
		glBindFramebuffer(GL_FRAMEBUFFER, frame_buffers_[kDefaultFrameBufferIdx]);
	}
	
	void RendererES2::EnableBlend(bool enable)
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
	
	void RendererES2::EnableMaterial(const MaterialData* data)
	{
		EnableDepthTest(data->depth_test);
		EnableDepthWrite(data->depth_write);
		EnableCullFace(data->cull_face, data->cull_front);
		EnableColorWrite(data->color_write);

		texture_enable_ = (data->used_unit > 0);
		
		for (int i = 0; i < MAX_TEXTURE_UNIT; ++i)
		{
			if (i < data->used_unit)
				EnableTextureUnit(i, data->texture_units[i]);
			else
				DisableTextureUnit(i);
		}
	}
	
	void RendererES2::EnableDepthTest(bool enable)
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
	
	void RendererES2::EnableDepthWrite(bool enable)
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
	
	void RendererES2::EnableCullFace(bool enable, bool cull_front)
	{
		if (cull_face_enable_ != enable)
		{
			cull_face_enable_ = enable;
			
			if (enable)
				glEnable(GL_CULL_FACE);
			else
				glDisable(GL_CULL_FACE);
		}
		
		if (cull_face_enable_ && cull_front_ != cull_front)
		{
			cull_front_ = cull_front;
			glCullFace(cull_front_ ? GL_FRONT : GL_BACK);
		}
	}
	
	void RendererES2::EnableColorWrite(ColorFlags enable)
	{
		if (color_write_enable_ != enable)
		{
			color_write_enable_ = enable;
			
			glColorMask(color_write_enable_.r,
						color_write_enable_.g,
						color_write_enable_.b,
						color_write_enable_.a);
		}
	}
	
	void RendererES2::EnableTextureUnit(int idx, const TextureUnit& unit)
	{
		GLenum tex_enum = GL_TEXTURE0 + idx;
		
		ActiveTextureUnit(tex_enum);
		
		if (unit.texture)
		{
			ASSERT(unit.texture->id);
			
			if (now_texture_ != unit.texture->id)
			{
				now_texture_ = unit.texture->id;
				glBindTexture(GL_TEXTURE_2D, now_texture_);
				
				glUniform1i(Root::Ins().shader_mgr()->current_program()->uniforms()[UNIFORM_TEX0 + idx], idx);
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
		
		texture_unit_coord_idx_[idx] = unit.coord_idx;
	}
	
	void RendererES2::DisableTextureUnit(int idx)
	{
		texture_unit_coord_idx_[idx] = -1;
	}
	
	void RendererES2::SetFog(FogMode mode, float density /*= 1.f*/)
	{
		fog_mode_ = mode;
		fog_density_ = density;
	}
	
	void RendererES2::SetFogDistance(float start, float end /*= 1.f*/)
	{
		fog_start_ = start;
		fog_end_ = end;
	}
	
	void RendererES2::SetFogColor(const Color& color)
	{
		fog_color_ = color;
	}
	
	unsigned int RendererES2::GenerateTexture(const void* buffer, int width, int height, PixelFormat format, int buffer_size /*= 0*/)
	{
		if (context_) context_->SetAsCurrent();

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
		
		// TODO: check support GL_IMG_texture_compression_pvrtc
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
#if ERI_PLATFORM == ERI_PLATFORM_IOS
			case RGBA_PVR_4BPP:
				glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG, width, height, 0, buffer_size, buffer);
				break;
			case RGBA_PVR_2BPP:
				glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG, width, height, 0, buffer_size, buffer);
				break;
#endif
			default:
				ASSERT2(0, "invalid pixel format!");
				break;
		}
		
		//glGenerateMipmap(GL_TEXTURE_2D);
		
		return texture;
	}
  
	unsigned int RendererES2::GenerateTexture()
	{
		if (context_) context_->SetAsCurrent();

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
		
		return texture;
	}
	
	void RendererES2::UpdateTexture(unsigned int texture_id, const void* buffer, int width, int height, PixelFormat format)
	{
		ASSERT(texture_id > 0);

		if (context_) context_->SetAsCurrent();
		
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
	
	void RendererES2::ReleaseTexture(int texture_id)
	{
		ASSERT(texture_id > 0);
		
		if (context_) context_->SetAsCurrent();

		if (now_texture_ == texture_id)
		{
			glBindTexture(GL_TEXTURE_2D, 0);
			now_texture_ = 0;
		}
		
		GLuint id = texture_id;
		glDeleteTextures(1, &id);
	}
	
	int RendererES2::GenerateFrameBuffer()
	{
		if (context_) context_->SetAsCurrent();
		
		for (int i = kDefaultFrameBufferIdx + 1; i < kMaxFrameBuffer; ++i)
		{
			if (!frame_buffers_[i])
			{
				glGenFramebuffers(1, &frame_buffers_[i]);
				return frame_buffers_[i];
			}
		}
		
		return 0;
	}
	
	void RendererES2::BindTextureToFrameBuffer(unsigned int texture_id, int frame_buffer)
	{
		ASSERT(texture_id > 0 && frame_buffer > 0);
		
		if (context_) context_->SetAsCurrent();
		
		glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
		
		glBindTexture(GL_TEXTURE_2D, texture_id);
		now_texture_ = texture_id;
		
		// attach the texture to the framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id, 0);
		
		// TODO: use depth buffer?
		
		//// allocate and attach a depth buffer
		//GLuint depth_render_buffer;
		//glGenRenderbuffersOES(1, &depth_render_buffer);
		//glBindRenderbufferOES(GL_RENDERBUFFER_OES, depth_render_buffer);
		//glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, width, height);
		//glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, depth_render_buffer);
		
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		ASSERT2(status == GL_FRAMEBUFFER_COMPLETE, "Failed to make complete framebuffer object %x", status);
	}
	
	void RendererES2::ReleaseFrameBuffer(int frame_buffer)
	{
		ASSERT(frame_buffer > 0);
		
		if (context_) context_->SetAsCurrent();
		
		for (int i = 0; i < kMaxFrameBuffer; ++i)
		{
			if (frame_buffers_[i] == frame_buffer)
			{
				glDeleteFramebuffers(1, &frame_buffers_[i]);
				frame_buffers_[i] = 0;
				return;
			}
		}
	}
	 
	void RendererES2::SetBgColor(const Color& color)
	{
		bg_color_ = color;
		glClearColor(bg_color_.r, bg_color_.g, bg_color_.b, bg_color_.a);
	}
	
	const Color& RendererES2::GetBgColor()
	{
		return bg_color_;
	}
	
	void RendererES2::SetClearDepth(float clamped_depth)
	{
#ifdef ERI_GL
		glClearDepth(clamped_depth);
#else
		glClearDepthf(clamped_depth);
#endif
	}
	
	void RendererES2::UpdateView(const Matrix4& view_matrix)
	{
		current_view_matrix_ = view_matrix;
		is_view_proj_dirty_ = true;
	}
	
	void RendererES2::UpdateView(const Vector3& eye, const Vector3& at, const Vector3& up)
	{
		MatrixLookAtRH(current_view_matrix_, eye, at, up);
		is_view_proj_dirty_ = true;
	}
	
	void RendererES2::UpdateProjection(const Matrix4& projection_matrix)
	{
		current_proj_matrix_ = projection_matrix;
		
		AdjustProjectionForViewOrientation();
		
		is_view_proj_dirty_ = true;
	}
	
	void RendererES2::UpdateOrthoProjection(float width, float height, float near_z, float far_z)
	{
		MatrixOrthoRH(current_proj_matrix_, width, height, near_z, far_z);
		
		AdjustProjectionForViewOrientation();
		
		is_view_proj_dirty_ = true;
	}
	
	void RendererES2::UpdateOrthoProjection(float zoom, float near_z, float far_z)
	{
		ASSERT(zoom);
		
		UpdateOrthoProjection(backing_width_ / zoom, backing_height_ / zoom, near_z, far_z);
	}
	
	void RendererES2::UpdatePerspectiveProjection(float fov_y, float aspect, float near_z, float far_z)
	{
		MatrixPerspectiveFovRH(current_proj_matrix_, fov_y, aspect, near_z, far_z);
		
		AdjustProjectionForViewOrientation();
		
		is_view_proj_dirty_ = true;
	}
	
	void RendererES2::UpdatePerspectiveProjection(float fov_y, float near_z, float far_z)
	{
		UpdatePerspectiveProjection(fov_y, static_cast<float>(backing_width_) / backing_height_, near_z, far_z);
	}
	
	void RendererES2::SetViewOrientation(ViewOrientation orientaion)
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
	
	void RendererES2::ActiveTextureUnit(GLenum idx)
	{
		if (now_active_texture_unit_ != idx)
		{
			now_active_texture_unit_ = idx;
			glActiveTexture(idx);
		}
	}
	
	void RendererES2::AdjustProjectionForViewOrientation()
	{
		static Matrix4 rot;
		
		switch (view_orientation_)
		{
			case PORTRAIT_HOME_BOTTOM:
				break;
			case PORTRAIT_HOME_TOP:
				Matrix4::RotateAxis(rot, 180.f, Vector3(0.f, 0.f, 1.f));
				current_proj_matrix_ = current_proj_matrix_ * rot;
				break;
			case LANDSCAPE_HOME_RIGHT:
				Matrix4::RotateAxis(rot, -90.f, Vector3(0.f, 0.f, 1.f));
				current_proj_matrix_ = current_proj_matrix_ * rot;
				break;
			case LANDSCAPE_HOME_LEFT:
				Matrix4::RotateAxis(rot, 90.f, Vector3(0.f, 0.f, 1.f));
				current_proj_matrix_ = current_proj_matrix_ * rot;
				break;
			default:
				ASSERT(0);
				break;
		}
	}

}

#endif // ERI_RENDERER_ES2
