//
//  Shader.vsh
//  gles_learn
//
//  Created by exe on 11/28/09.
//  Copyright cobbler 2009. All rights reserved.
//

attribute vec4 position;
attribute vec4 color;

varying vec4 colorVarying;

uniform float translate;

void main()
{
	gl_Position = position;
	gl_Position.y += sin(translate) / 2.0;
	
	colorVarying = color;
}
