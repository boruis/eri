/*
 *  render_data.cpp
 *  eri
 *
 *  Created by exe on 11/29/09.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#include "pch.h"

#include "render_data.h"

namespace ERI {
	
	RenderData::RenderData() :
		vertex_buffer(0),
		vertex_type(GL_TRIANGLE_STRIP),
		vertex_format(POS_TEX_2),
		vertex_count(0),
		index_buffer(0),
		index_count(0),
		scale(Vector3(1, 1, 1)),
		rotate_axis(Vector3(0, 0, 1)),
		rotate_degree(0),
		need_update_model_matrix(true),
		need_update_world_model_matrix(true),
		need_update_inv_world_model_matrix(true),
		apply_identity_model_matrix(false),
		is_tex_transform(false),
		tex_scale(Vector2(1, 1)),
		blend_src_factor(GL_SRC_ALPHA),
		blend_dst_factor(GL_ONE_MINUS_SRC_ALPHA),
		alpha_test_func(GL_GREATER),
		alpha_test_ref(0.0f),
		depth_test_func(GL_LESS),
		program(NULL)
	{
	}
	
	RenderData::~RenderData()
	{
		if (vertex_buffer != 0)
		{
			glDeleteBuffers(1, &vertex_buffer);
		}
		if (index_buffer != 0)
		{
			glDeleteBuffers(1, &index_buffer);
		}
	}
	
	void RenderData::UpdateModelMatrix()
	{
		ASSERT(need_update_model_matrix);
		
		need_update_model_matrix = false;
		
		Matrix4 tmp;
		
		Matrix4::Translate(model_matrix, translate);
		Matrix4::RotateAxis(tmp, rotate_degree, rotate_axis);
		model_matrix = model_matrix * tmp;
		Matrix4::Scale(tmp, scale);
		model_matrix = model_matrix * tmp;
		
		Matrix4::Inverse(inv_model_matrix, model_matrix);
	}
	
	void RenderData::UpdateWorldModelMatrix(const Matrix4& parent_world_model_matrix)
	{
		ASSERT(!need_update_model_matrix);
		ASSERT(need_update_world_model_matrix);
		
		need_update_world_model_matrix = false;
		
		world_model_matrix = parent_world_model_matrix * model_matrix;
	}
	
	void RenderData::UpdateWorldModelMatrix()
	{
		ASSERT(!need_update_model_matrix);
		ASSERT(need_update_world_model_matrix);
		
		need_update_world_model_matrix = false;
		
		world_model_matrix = model_matrix;
	}

	void RenderData::UpdateInvWorldModelMatrix()
	{
		ASSERT(!need_update_world_model_matrix);
		ASSERT(need_update_inv_world_model_matrix);
		
		need_update_inv_world_model_matrix = false;
		
		Matrix4::Inverse(inv_world_model_matrix, world_model_matrix);
	}
}
