/*
 *  mesh_actor.cpp
 *  eri
 *
 *  Created by exe on 2010/10/26.
 *  Copyright 2010 cobbler. All rights reserved.
 *
 */

#include "pch.h"

#include "mesh_actor.h"

#include "collada_loader.h"

namespace ERI
{
	
	MeshActor::MeshActor(MeshLoader* loader)
	{
		UpdateVertexBuffer(loader);
	}
	
	MeshActor::~MeshActor()
	{
	}
	
	void MeshActor::UpdateVertexBuffer(MeshLoader* loader)
	{
		if (render_data_.vertex_buffer == 0)
		{
			glGenBuffers(1, &render_data_.vertex_buffer);
		}
		
		int vertex_buffer_size = loader->GetVertexBufferSize();
		
		ASSERT(vertex_buffer_size > 0);
		
		void* vertex_buffer = malloc(vertex_buffer_size);
		render_data_.vertex_count = loader->FillVertexBuffer(vertex_buffer);
		
		glBindBuffer(GL_ARRAY_BUFFER, render_data_.vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size, vertex_buffer, GL_STATIC_DRAW);
		
		free(vertex_buffer);

		loader->GetVertexInfo(render_data_.vertex_type, render_data_.vertex_format);
		
		//
		
		int index_buffer_size = loader->GetIndexBufferSize();
		if (index_buffer_size > 0)
		{
			if (render_data_.index_buffer == 0)
			{
				glGenBuffers(1, &render_data_.index_buffer);
			}
			
			void* index_buffer = malloc(index_buffer_size);
			render_data_.index_count = loader->FillIndexBuffer(index_buffer);
			
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_data_.index_buffer);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_size, index_buffer, GL_STATIC_DRAW);
			
			free(index_buffer);
		}
	}

}
