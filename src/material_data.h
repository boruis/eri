/*
 *  material_data.h
 *  eri
 *
 *  Created by exe on 12/22/09.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#ifndef ERI_MATERIAL_DATA_H
#define ERI_MATERIAL_DATA_H

#include "texture_mgr.h"

#define MAX_TEXTURE_UNIT	4

namespace ERI {
	
	enum OpacityType
	{
		OPACITY_OPAQUE,
		OPACITY_ALPHA_TEST,
		OPACITY_ALPHA_BLEND
	};
	
	struct TextureUnit
	{
		TextureUnit() : texture(NULL) {}
		
		const Texture*	texture;
		TextureParams	params;
		TextureEnvs		envs;
	};
	
	struct MaterialData
	{
		MaterialData() :
			used_unit(0),
			opacity_type(OPACITY_ALPHA_BLEND),
			depth_test(true),
			depth_write(true),
			accept_light(false)
		{
		}
		
		TextureUnit		texture_units[MAX_TEXTURE_UNIT];
		int				used_unit;

		OpacityType		opacity_type;
		bool			depth_test;
		bool			depth_write;
		bool			accept_light;
	};
	
}

#endif // ERI_MATERIAL_DATA_H
