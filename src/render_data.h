/*
 *  render_data.h
 *  eri
 *
 *  Created by exe on 11/29/09.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#ifndef ERI_RENDER_DATA_H
#define ERI_RENDER_DATA_H

#include "pch.h"

#include "math_helper.h"

namespace ERI {
	
	struct MaterialData;
	class ShaderProgram;
	
	struct vertex_2_pos_tex {
		GLfloat position[2];
		GLfloat tex_coord[2];
	};

	struct vertex_2_pos_tex2 {
		GLfloat position[2];
		GLfloat tex_coord[2];
		GLfloat tex_coord2[2];
	};
	
	struct vertex_2_pos_tex_color {
		GLfloat position[2];
		GLbyte	color[4];
		GLfloat tex_coord[2];
	};

	struct vertex_2_pos_tex2_color {
		GLfloat position[2];
		GLbyte	color[4];
		GLfloat tex_coord[2];
		GLfloat tex_coord2[2];
	};
	
	struct vertex_3_pos_normal {
		GLfloat position[3];
		GLfloat normal[3];
	};
	
	struct vertex_3_pos_normal_tex {
		GLfloat position[3];
		GLfloat normal[3];
		GLfloat tex_coord[2];
	};
	
	struct vertex_3_pos_normal_color_tex {
		GLfloat position[3];
		GLfloat normal[3];
		GLbyte	color[4];
		GLfloat tex_coord[2];
	};
	
	struct vertex_3_pos_color_tex {
		GLfloat position[3];
		GLbyte	color[4];
		GLfloat tex_coord[2];
	};
	
	enum VertexFormat
	{
		POS_TEX_2 = 0,
		POS_TEX2_2,
		POS_TEX_COLOR_2,
		POS_TEX2_COLOR_2,
		POS_NORMAL_3,
		POS_NORMAL_TEX_3,
		POS_NORMAL_COLOR_TEX_3,
		POS_COLOR_TEX_3,
		VERTEX_FORMAT_MAX
	};

	struct RenderData
	{
		RenderData();
		~RenderData();
		
		void UpdateModelMatrix();
		void UpdateWorldModelMatrix(const Matrix4& parent_world_model_matrix);
		void UpdateWorldModelMatrix();
		void UpdateInvWorldModelMatrix();

		// model
		mutable GLuint vertex_array; // create by renderer
		
		GLuint			vertex_buffer;
		GLenum			vertex_type;
		VertexFormat	vertex_format;
		int				vertex_count;
		
		GLuint			index_buffer;
		int				index_count;
		
		Vector3	translate;
		Vector3	scale;
		// TODO: use Quaternion info?
		Vector3 rotate_axis;
		float	rotate_degree;
		
		Matrix4	model_matrix;
		Matrix4	inv_model_matrix;
		Matrix4	world_model_matrix;
		Matrix4	inv_world_model_matrix;
		bool	need_update_model_matrix;
		bool	need_update_world_model_matrix, need_update_inv_world_model_matrix;
		bool	apply_identity_model_matrix;
		
		Vector3	world_view_pos;
		
		// TODO: move to MaterialData?
		
		// texture
		// TODO: multi-texture?
		bool	is_tex_transform;
		Vector2	tex_scale;
		Vector2 tex_translate;
		
		Color	color;
		
		GLenum blend_src_factor, blend_dst_factor;
		bool alpha_premultiplied;
		
		GLenum		alpha_test_func;
		GLclampf	alpha_test_ref;

		GLenum	depth_test_func;
    
		ShaderProgram* program;
		
		const MaterialData* material_ref;
	};

}

#endif // ERI_RENDER_DATA_H
