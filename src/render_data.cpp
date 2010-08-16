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
		
		Matrix4::Scale(model_matrix, scale);
		
		Matrix4::RotateAxis(tmp, rotate_degree, rotate_axis);
		model_matrix = model_matrix * tmp;
		
		Matrix4::Translate(tmp, translate);
		model_matrix = model_matrix * tmp;
		
		Matrix4::Inverse(inv_model_matrix, model_matrix);
	}
	
	void RenderData::UpdateWorldModelMatrix(const Matrix4& parent_world_model_matrix)
	{
		ASSERT(!need_update_model_matrix);
		ASSERT(need_update_world_model_matrix);
		
		need_update_world_model_matrix = false;
		
		world_model_matrix = model_matrix * parent_world_model_matrix;
	}
	
	void RenderData::UpdateWorldModelMatrix()
	{
		ASSERT(!need_update_model_matrix);
		ASSERT(need_update_world_model_matrix);
		
		need_update_world_model_matrix = false;
		
		world_model_matrix = model_matrix;
	}

}
