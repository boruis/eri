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

namespace ERI {
	
	enum OpacityType
	{
		OPACITY_OPAQUE,
		OPACITY_ALPHA_TEST,
		OPACITY_ALPHA_BLEND
	};
	
	struct MaterialData
	{
		MaterialData() :
			texture(NULL),
			tex_filter_min(FILTER_NEAREST),
			tex_filter_mag(FILTER_NEAREST),
			opacity_type(OPACITY_ALPHA_BLEND),
			depth_test(true),
			depth_write(true),
			accept_light(false)
		{
		}
		
		const Texture*	texture;
		TextureFilter	tex_filter_min;
		TextureFilter	tex_filter_mag;

		OpacityType		opacity_type;
		bool			depth_test;
		bool			depth_write;
		
		bool			accept_light;
	};
	
}

#endif // ERI_MATERIAL_DATA_H
