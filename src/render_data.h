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

#if ERI_PLATFORM == ERI_PLATFORM_WIN
#include "GL/glew.h"
//#include "GL/wglew.h"
//#include "GLee.h"
#elif ERI_PLATFORM == ERI_PLATFORM_MAC
#include <OpenGL/gl.h>
#elif ERI_PLATFORM == ERI_PLATFORM_ANDROID
#include <GLES/gl.h>
#elif ERI_PLATFORM == ERI_PLATFORM_IOS
#import <OpenGLES/ES1/gl.h>
//#import <OpenGLES/ES2/gl.h>
#endif

namespace ERI {
	
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
		POS_TEX_2,
		POS_TEX2_2,
		POS_TEX_COLOR_2,
		POS_NORMAL_3,
		POS_NORMAL_TEX_3,
		POS_NORMAL_COLOR_TEX_3,
		POS_COLOR_TEX_3,
		INVALID_VERTEX_FORMAT
	};

	struct RenderData
	{
		RenderData();
		~RenderData();
		
		void UpdateModelMatrix();
		void UpdateWorldModelMatrix(const Matrix4& parent_world_model_matrix);
		void UpdateWorldModelMatrix();
		
		// model
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
		bool	need_update_model_matrix;
		bool	need_update_world_model_matrix;
		bool	apply_identity_model_matrix;
		
		Vector3	world_view_pos;
		
		// texture
		// TODO: multi-texture?
		bool	is_tex_transform;
		Vector2	tex_scale;
		Vector2 tex_translate;
		
		// color
		Color	color;
		
		// blend
		GLenum blend_src_factor, blend_dst_factor;
		
		// alpha test
		GLenum		alpha_test_func;
		GLclampf	alpha_test_ref;
	};

}

#endif // ERI_RENDER_DATA_H
