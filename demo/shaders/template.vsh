#define NUM_TEXTURES 2

const int i_zero = 0;
const int i_one = 1;

uniform mat4 model_view_proj_matrix;
uniform bool tex_enable[NUM_TEXTURES];
uniform bool tex_matrix_enable[NUM_TEXTURES];
uniform mat4 tex_matrix[NUM_TEXTURES];

attribute vec4 a_position;
attribute vec4 a_color;
attribute vec2 a_texcoord0;
attribute vec2 a_texcoord1;

varying vec4 v_color;
varying vec2 v_texcoord[NUM_TEXTURES];

void main()
{
	gl_Position = model_view_proj_matrix * a_position;
	v_color = a_color;

	if (tex_enable[i_zero])
	{
		if (tex_matrix_enable[i_zero])
		{
			v_texcoord[i_zero] = (tex_matrix[i_zero] * vec4(a_texcoord0, 0.0, 0.0)).st;
		}
		else
		{
			v_texcoord[i_zero] = a_texcoord0;
		}
	}

	if (tex_enable[i_one])
	{
		if (tex_matrix_enable[i_one])
			v_texcoord[i_one] = (tex_matrix[i_one] * vec4(a_texcoord1, 0.0, 0.0)).st;
		else
			v_texcoord[i_one] = a_texcoord1;
	}
}
