/*
 *  collada_loader.cpp
 *  eri
 *
 *  Created by exe on 2010/10/25.
 *  Copyright 2010 cobbler. All rights reserved.
 *
 */

#include "pch.h"

#include "collada_loader.h"

#include <fstream>
#include <sstream>

#include "xml_helper.h"

using namespace rapidxml;

namespace ERI
{
	Triangles::~Triangles()
	{
		for (int i = 0; i < inputs.size(); ++i)
		{
			delete inputs[i];
		}
	}
	
	Mesh::~Mesh()
	{
		std::map<std::string, Src*>::iterator src_it = src_map.begin();
		for (; src_it != src_map.end(); ++src_it)
		{
			delete src_it->second;
		}

		std::map<std::string, Input*>::iterator vertices_it = vertices_map.begin();
		for (; vertices_it != vertices_map.end(); ++vertices_it)
		{
			delete vertices_it->second;
		}

		for (int i = 0; i < triangles_array.size(); ++i)
		{
			delete triangles_array[i];
		}
	}

	ColladaLoader::ColladaLoader(const std::string& path) : current_load_mesh_(NULL)
	{
		std::ifstream ifs;
		ifs.open(path.c_str());
		
		if (ifs.fail())
		{
			return;
		}
		
		ifs.seekg(0, std::ios::end);
		int length = ifs.tellg();
		ifs.seekg(0, std::ios::beg);
		
		if (length == 0)
		{
			ifs.close();
			return;
		}
		
		char* buffer = new char[length + 1];
		memset(buffer, 0, length + 1);
		
		ifs.read(buffer, length);
		ifs.close();
		
		xml_document<> doc;
		xml_node<>* node;
		xml_node<>* node2;
		xml_node<>* node3;
		xml_node<>* node4;
		std::string s, s2;
		
		doc.parse<0>(buffer);
		
		std::string geometry_id;
		Mesh* geometry = NULL;
		
		node = doc.first_node("COLLADA");
		if (!node)
			return;

		node = node->first_node();
		while (node)
		{
			if (strcmp(node->name(), "library_geometries") == 0)
			{
				node2 = node->first_node();
				while (node2)
				{
					printf("%s\n", node2->name());
					
					if (strcmp(node2->name(), "geometry") == 0)
					{
						if (GetAttrStr(node2, "id", s))
						{
							if (geometry)
								geometry_map_[geometry_id] = geometry;

							geometry = new Mesh;
							geometry_id = s;
							
							// TODO: check already exist?
						}
						else
						{
							node2 = node2->next_sibling();
							continue;
						}
					}
					else
					{
						node2 = node2->next_sibling();
						continue;
					}
					
					// current support mesh only
					node3 = node2->first_node("mesh");
					if (!node3)
					{
						node2 = node2->next_sibling();
						continue;
					}
					
					node3 = node3->first_node();
					while (node3)
					{
						if (strcmp(node3->name(), "source") == 0)
						{
							if (!GetAttrStr(node3, "id", s))
								continue;
							
							node4 = node3->first_node("float_array");
							if (!node4)
								continue;
							
							s2 = node4->value();
							std::istringstream iss(s2);
							
							Src* src = new Src;
							
							float value;
							while (!iss.eof())
							{
								iss >> value;
								src->datas.push_back(value);
							}
							
							node4 = node3->first_node("technique_common");
							if (!node4)
							{
								delete src;
								continue;
							}
							
							node4 = node4->first_node("accessor");
							if (!node4)
							{
								delete src;
								continue;
							}
							
							if (!GetAttrInt(node4, "count", src->count))
							{
								delete src;
								continue;
							}

							if (!GetAttrInt(node4, "stride", src->stride))
							{
								delete src;
								continue;
							}
							
							// TODO: check already exist?
							
							geometry->src_map[s] = src;
						}
						else if (strcmp(node3->name(), "vertices") == 0)
						{
							if (!GetAttrStr(node3, "id", s))
								continue;
							
							node4 = node3->first_node("input");
							if (!node4)
								continue;
							
							Input* input = ParseInput(node4);
							if (input)
							{
								// TODO: check already exist?
								
								geometry->vertices_map[s] = input;
							}
						}
						else if (strcmp(node3->name(), "triangles") == 0)
						{
							Triangles* tris = new Triangles;
							
							GetAttrInt(node3, "count", tris->count);
							GetAttrStr(node3, "material", tris->material);
							
							node4 = node3->first_node();
							while (node4)
							{
								if (strcmp(node4->name(), "input") == 0)
								{
									Input* input = ParseInput(node4);
									if (input)
									{
										tris->inputs.push_back(input);
									}
								}
								else if (strcmp(node4->name(), "p") == 0)
								{
									s = node4->value();
									std::istringstream iss(s);
									
									float value;
									while (!iss.eof())
									{
										iss >> value;
										tris->primitives.push_back(value);
									}
								}
								
								node4 = node4->next_sibling();
							}
							
							if (tris->inputs.empty() || tris->primitives.empty())
							{
								delete tris;
								continue;
							}
							
							geometry->triangles_array.push_back(tris);
						}
						
						node3 = node3->next_sibling();
					}
					
					node2 = node2->next_sibling();
				}
				
				if (geometry)
					geometry_map_[geometry_id] = geometry;
			}
			
			node = node->next_sibling();
		}
		
		delete [] buffer;
	}

	ColladaLoader::~ColladaLoader()
	{
		std::map<std::string, Mesh*>::iterator it = geometry_map_.begin();
		for (; it != geometry_map_.end(); ++it)
		{
			delete it->second;
		}
	}
	
	void ColladaLoader::SetCurrentMesh(const std::string& name)
	{
		current_load_mesh_ = NULL;
		
		std::map<std::string, Mesh*>::iterator it = geometry_map_.find(name);
		if (it != geometry_map_.end())
		{
			current_load_mesh_ = it->second;
		}
		else if (!geometry_map_.empty())
		{
			current_load_mesh_ = geometry_map_.begin()->second;
		}
	}
	
	int ColladaLoader::GetVertexBufferSize()
	{
		if (!current_load_mesh_ || current_load_mesh_->triangles_array.empty())
			return 0;
		
		bool have_pos, have_normal, have_texcoord, have_color;
		have_pos = have_normal = have_texcoord = have_color = false;

		// TODO: will all triangles have different inputs?
		// TODO: support tristrips? polygons? ...
		
		Triangles* tri = current_load_mesh_->triangles_array[0];
		current_vertex_type_ = GL_TRIANGLES;
		
		for (int i = 0; i < tri->inputs.size(); ++i)
		{
			if (tri->inputs[i]->semantic == VERTEX)
			{
				have_pos = true;
			}
			else if (tri->inputs[i]->semantic == NORMAL)
			{
				have_normal = true;
			}
			else if (tri->inputs[i]->semantic == TEXCOORD)
			{
				have_texcoord = true;
			}
			else
			{
				ASSERT(0);
			}
		}
		
		int tri_total_count = 0;
		for (int i = 0; i < current_load_mesh_->triangles_array.size(); ++i)
		{
			tri_total_count += current_load_mesh_->triangles_array[i]->count;
		}

		if (have_pos && have_normal && have_texcoord)
		{
			current_vertex_format_ = POS_NORMAL_TEX_3;
			current_vertex_size_ = sizeof(vertex_3_pos_normal_tex);
			return current_vertex_size_ * 3 * tri_total_count;
		}
		else if (have_pos && have_normal)
		{
			current_vertex_format_ = POS_NORMAL_3;
			current_vertex_size_ = sizeof(vertex_3_pos_normal);
			return current_vertex_size_ * 3 * tri_total_count;
		}
		else
		{
			ASSERT(0);
		}

		return 0;
	}
	
	int ColladaLoader::FillVertexBuffer(void* buffer)
	{
		ASSERT(current_load_mesh_ && !current_load_mesh_->triangles_array.empty());
		
		// TODO: will all triangles have different inputs?
		// TODO: support tristrips? polygons? ...
		
		unsigned char* buffer_data = static_cast<unsigned char*>(buffer);
		int vertex_total_num = 0;
		
		for (int tri_idx = 0; tri_idx < current_load_mesh_->triangles_array.size(); ++tri_idx)
		{
			Triangles* tri = current_load_mesh_->triangles_array[tri_idx];
			int input_num = tri->inputs.size();
			
			std::vector<Src*> input_srcs;
			Src* src;
			for (int i = 0; i < input_num; ++i)
			{
				if (tri->inputs[i]->semantic == VERTEX)
				{
					Input* input = current_load_mesh_->vertices_map[tri->inputs[i]->src];
					src = current_load_mesh_->src_map[input->src];
				}
				else
				{
					src = current_load_mesh_->src_map[tri->inputs[i]->src];
				}
				input_srcs.push_back(src);
			}
			
			int vertex_num = tri->count * 3;
			int vertex_stride = tri->primitives.size() / vertex_num;
			
			ASSERT((vertex_stride * vertex_num) == tri->primitives.size());
			
			int buffer_offset;
			int src_idx, src_stride, src_want_stride;
			
			for (int i = 0; i < vertex_num; ++i)
			{
				buffer_offset = 0;
				
				for (int j = 0; j < input_num; ++j)
				{
					src_idx = tri->primitives[i * vertex_stride + tri->inputs[j]->offset];
					src_stride = src_want_stride = input_srcs[j]->stride;
					
					// TODO: check this, some resource's TEXCOORD stride will greater than 2 (S,T,P), we can only use 2
					if (tri->inputs[j]->semantic == TEXCOORD)
						src_want_stride = 2;
					
					for (int k = 0; k < src_want_stride; ++k)
					{
						memcpy(&buffer_data[buffer_offset + k * 4],
							   &input_srcs[j]->datas[src_idx * src_stride + k],
							   4);
						
						ASSERT(reinterpret_cast<long>(&buffer_data[buffer_offset + k * 4]) < reinterpret_cast<long>(buffer_data) + current_vertex_size_);
					}
					
					buffer_offset += input_srcs[j]->stride * 4;
				}
				
				buffer_data += current_vertex_size_;
			}
			
			vertex_total_num += vertex_num;
		}
		
		return vertex_total_num;
	}
	
	int ColladaLoader::GetIndexBufferSize()
	{
		return 0;
	}
	
	int ColladaLoader::FillIndexBuffer(void* buffer)
	{
		return 0;
	}
	
	void ColladaLoader::GetVertexInfo(GLenum& vertex_type, VertexFormat& vertex_format)
	{
		vertex_type = current_vertex_type_;
		vertex_format = current_vertex_format_;
	}

	Input* ColladaLoader::ParseInput(rapidxml::xml_node<>* node)
	{
		std::string s;
		
		Input* input = new Input;
		
		if (!GetAttrStr(node, "semantic", s))
		{
			delete input;
			return NULL;
		}
		
		bool is_need_offset = true;
		bool is_need_set = false;
		
		if (s.compare("POSITION") == 0)
		{
			input->semantic = POSITION;
			is_need_offset = false;
		}
		else if (s.compare("VERTEX") == 0)
		{
			input->semantic = VERTEX;
		}
		else if (s.compare("NORMAL") == 0)
		{
			input->semantic = NORMAL;
		}
		else if (s.compare("TEXCOORD") == 0)
		{
			input->semantic = TEXCOORD;
			is_need_set = true;
		}
		else
		{
			delete input;
			return NULL;
		}
		
		if (GetAttrStr(node, "source", input->src))
		{
			if (input->src[0] == '#')
				input->src = input->src.substr(1);
		}
		else
		{
			delete input;
			return NULL;
		}
		
		if (is_need_offset && !GetAttrInt(node, "offset", input->offset))
		{
			delete input;
			return NULL;
		}
		
		if (is_need_set && !GetAttrInt(node, "set", input->set))
		{
			delete input;
			return NULL;
		}

		return input;
	}

}
