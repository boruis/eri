/*
 *  mesh_loader.h
 *  eri
 *
 *  Created by exe on 2010/10/27.
 *  Copyright 2010 cobbler. All rights reserved.
 *
 */

#ifndef ERI_MESH_LOADER_H
#define ERI_MESH_LOADER_H

#include "render_data.h"

namespace ERI
{
	
	class MeshLoader
	{
	public:
		virtual ~MeshLoader() {}
		
		virtual int GetVertexBufferSize() = 0;
		virtual int FillVertexBuffer(void* buffer) = 0;
		
		virtual int GetIndexBufferSize() = 0;
		virtual int FillIndexBuffer(void* buffer) = 0;
		
		virtual void GetVertexInfo(GLenum& vertex_type, VertexFormat& vertex_format) = 0;
	};
	
}

#endif // ERI_MESH_LOADER_H
