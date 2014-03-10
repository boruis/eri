/*
 *  mesh_actor.h
 *  eri
 *
 *  Created by exe on 2010/10/26.
 *  Copyright 2010 cobbler. All rights reserved.
 *
 */

#ifndef ERI_MESH_ACTOR
#define ERI_MESH_ACTOR

#include "scene_actor.h"

namespace ERI
{
	class MeshLoader;
	
	class MeshActor : public SceneActor
	{
	public:
		MeshActor(MeshLoader* loader);
		virtual ~MeshActor();
		
	private:
		void UpdateVertexBuffer(MeshLoader* loader);
	};
	
}

#endif // ERI_MESH_ACTOR
