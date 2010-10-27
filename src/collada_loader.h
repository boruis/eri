/*
 *  collada_loader.h
 *  eri
 *
 *  Created by exe on 2010/10/25.
 *  Copyright 2010 cobbler. All rights reserved.
 *
 */

#ifndef ERI_COLLADA_LOADER
#define ERI_COLLADA_LOADER

#include <string>
#include <vector>
#include <map>

#include "rapidxml.hpp"

#include "mesh_loader.h"

namespace ERI
{

	enum Semantic
	{
		VERTEX,
		POSITION,
		NORMAL,
		TEXCOORD
	};
	
	struct Src
	{
		std::vector<float>	datas;
		int					count;
		int					stride;
	};
	
	struct Input
	{
		Semantic		semantic;
		std::string		src;
		int				offset;
		int				set;
	};
	
	struct Triangles
	{
		~Triangles();
		
		std::vector<Input*>		inputs;
		std::vector<float>		primitives;
		std::string				material;
		int						count;
	};

	struct Mesh
	{
		~Mesh();
		
		std::map<std::string, Src*>		src_map;
		std::map<std::string, Input*>	vertices_map;
		std::vector<Triangles*>			triangles_array;
	};
	
	class ColladaLoader : public MeshLoader
	{
	public:
		ColladaLoader(const std::string& path);
		virtual ~ColladaLoader();
		
		// MeshLoader
		
		void SetCurrentMesh(const std::string& name);
		
		virtual int GetVertexBufferSize();
		virtual int FillVertexBuffer(void* buffer);
		
		virtual int GetIndexBufferSize();
		virtual int FillIndexBuffer(void* buffer);
		
		virtual void GetVertexInfo(GLenum& vertex_type, VertexFormat& vertex_format);
		
	private:
		Input* ParseInput(rapidxml::xml_node<>* node);
		
		std::map<std::string, Mesh*>	geometry_map_;
		
		Mesh*							current_load_mesh_;
		GLenum							current_vertex_type_;
		VertexFormat					current_vertex_format_;
		int								current_vertex_size_;
	};

}

#endif // ERI_COLLADA_LOADER
