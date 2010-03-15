//
//  Shader.fsh
//  gles_learn
//
//  Created by exe on 11/28/09.
//  Copyright cobbler 2009. All rights reserved.
//

varying lowp vec4 colorVarying;

void main()
{
	gl_FragColor = colorVarying;
}
