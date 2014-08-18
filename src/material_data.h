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
		TextureUnit() : texture(NULL), coord_idx(0) {}
		
		const Texture*	texture;
		TextureParams	params;
		TextureEnvs		envs;
		int coord_idx;
	};
	
	struct ColorFlags
	{
		ColorFlags() : r(true), g(true), b(true), a(true) {}
		
		inline bool operator == (const ColorFlags& rhs)
		{
			return (r == rhs.r &&
					g == rhs.g &&
					b == rhs.b &&
					a == rhs.a);
		}
		
		inline bool operator != (const ColorFlags& rhs)
		{
			return (r != rhs.r ||
					g != rhs.g ||
					b != rhs.b ||
					a != rhs.a);
		}
		
		unsigned int r : 1;
		unsigned int g : 1;
		unsigned int b : 1;
		unsigned int a : 1;
	};
	
	struct MaterialData
	{
		MaterialData() :
			used_unit(0),
			opacity_type(OPACITY_ALPHA_BLEND),
			depth_test(true),
			depth_write(true),
			cull_face(true),
			cull_front(false),
			accept_light(false),
			accept_fog(false)
		{
		}
		
		int GetSingleTextureId() const
		{
			int texture_id = 0;
			
			for (int i = 0; i < MAX_TEXTURE_UNIT; ++i)
			{
				if (texture_units[i].texture)
				{
					if (texture_id)
					{
						return 0; // if have multiple texture, no single texture id
					}
					
					texture_id = texture_units[i].texture->id;
				}
			}
			
			return texture_id;
		}
		
		TextureUnit		texture_units[MAX_TEXTURE_UNIT];
		int				used_unit;

		OpacityType		opacity_type;
		bool			depth_test;
		bool			depth_write;
		bool			cull_face;
		bool			cull_front;
		bool			accept_light;
		bool			accept_fog;
		
		ColorFlags		color_write;
	};
	
}

#endif // ERI_MATERIAL_DATA_H
