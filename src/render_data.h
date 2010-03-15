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

#include "math_helper.h"

#ifdef OS_ANDROID
#include <GLES/gl.h>
#else
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES2/gl.h>
#endif

namespace ERI {
	
	struct vertex_2_pos_tex {
		GLfloat position[2];
		GLfloat tex_coord[2];
	};

	struct RenderData
	{
		RenderData() :
			vertex_buffer(0),
			vertex_type(GL_TRIANGLE_STRIP),
			vertex_count(0),
			scale(Vector3(1, 1, 1)),
			rotate_axis(Vector3(0, 0, 1)),
			rotate_degree(0),
			need_update_model_matrix(true),
			need_update_world_model_matrix(true),
			is_tex_transform(false)
		{
		}
		
		void UpdateModelMatrix();
		void UpdateWorldModelMatrix(const Matrix4& parent_world_model_matrix);
		void UpdateWorldModelMatrix();
		
		// model
		GLuint	vertex_buffer;
		GLenum	vertex_type;
		int		vertex_count;
		Vector3	translate;
		Vector3	scale;
		Vector3 rotate_axis;
		float	rotate_degree;
		Matrix4	model_matrix;
		Matrix4	inv_model_matrix;
		Matrix4	world_model_matrix;
		bool	need_update_model_matrix;
		bool	need_update_world_model_matrix;
		
		// texture
		bool	is_tex_transform;
		Vector2	tex_scale;
		Vector2 tex_translate;
		
		// color
		Color	color;
	};

}

#endif // ERI_RENDER_DATA_H
