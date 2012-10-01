#define NUM_TEXTURES 2

const int i_zero = 0;
const int i_one = 1;

#ifdef GL_ES
// define default precision for float, vec, mat.
precision highp float;
#endif

uniform bool tex_enable[NUM_TEXTURES];
uniform sampler2D tex[NUM_TEXTURES];

varying vec4 v_color;
varying vec2 v_texcoord[NUM_TEXTURES];

void main()
{
	gl_FragColor = v_color;

	if (tex_enable[i_zero])
		gl_FragColor *= texture2D(tex[i_zero], v_texcoord[i_zero]);

	if (tex_enable[i_one])
		gl_FragColor *= texture2D(tex[i_one], v_texcoord[i_one]);
}
