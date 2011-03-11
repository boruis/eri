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
#include "skeleton_actor.h"

using namespace rapidxml;

namespace ERI
{
	
#pragma mark ColladaLoader::Triangles
	
	ColladaLoader::Triangles::~Triangles()
	{
		for (int i = 0; i < inputs.size(); ++i)
		{
			delete inputs[i];
		}
	}
	
#pragma mark ColladaLoader::Mesh
	
	ColladaLoader::Mesh::~Mesh()
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
	
#pragma mark ColladaLoader::Skin
	
	ColladaLoader::Skin::~Skin()
	{
		std::map<std::string, Src*>::iterator src_it = src_map.begin();
		for (; src_it != src_map.end(); ++src_it)
		{
			delete src_it->second;
		}
		
		for (int i = 0; i < joint_array.size(); ++i)
		{
			delete joint_array[i];
		}
		
		for (int i = 0; i < vertex_weight_array.size(); ++i)
		{
			delete vertex_weight_array[i];
		}
	}
	
#pragma mark ColladaLoader::Skeleton
	
	ColladaLoader::Skeleton::~Skeleton()
	{
		for (int i = 0; i < nodes.size(); ++i)
		{
			delete nodes[i];
		}
	}
	
#pragma mark ColladaLoader::Animation
	
	ColladaLoader::Animation::~Animation()
	{
		std::map<std::string, Src*>::iterator src_it = src_map.begin();
		for (; src_it != src_map.end(); ++src_it)
		{
			delete src_it->second;
		}
		
		std::map<std::string, Sampler*>::iterator sampler_it = sampler_map.begin();
		for (; sampler_it != sampler_map.end(); ++sampler_it)
		{
			delete sampler_it->second;
		}
		
		for (int i = 0; i < channel_array.size(); ++i)
		{
			delete channel_array[i];
		}
	}
	
#pragma mark ColladaLoader

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
		
		doc.parse<0>(buffer);
		
		node = doc.first_node("COLLADA");
		if (node)
		{
			node = node->first_node();
			while (node)
			{
				if (strcmp(node->name(), "library_geometries") == 0)
				{
					ParseLibraryGeometries(node);
				}
				else if (strcmp(node->name(), "library_controllers") == 0)
				{
					ParseLibraryControllers(node);
				}
				else if (strcmp(node->name(), "library_visual_scenes") == 0)
				{
					ParseLibraryVisualScenes(node);
				}
				else if (strcmp(node->name(), "library_animations") == 0)
				{
					ParseLibraryAnimations(node);
				}
				
				node = node->next_sibling();
			}
		}
		
		delete [] buffer;
	}

	ColladaLoader::~ColladaLoader()
	{
		std::map<std::string, Mesh*>::iterator geometry_it = geometry_map_.begin();
		for (; geometry_it != geometry_map_.end(); ++geometry_it)
		{
			delete geometry_it->second;
		}
		
		std::map<std::string, Skin*>::iterator skin_it = skin_map_.begin();
		for (; skin_it != skin_map_.end(); ++skin_it)
		{
			delete skin_it->second;
		}

		std::map<std::string, Skeleton*>::iterator skel_it = skeleton_map_.begin();
		for (; skel_it != skeleton_map_.end(); ++skel_it)
		{
			delete skel_it->second;
		}
		
		for (int i = 0; i < animation_array_.size(); ++i)
		{
			delete animation_array_[i];
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
			else if (tri->inputs[i]->semantic == COLOR)
			{
				have_color = true;
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

		if (have_pos && have_normal && have_texcoord && have_color)
		{
			current_vertex_format_ = POS_NORMAL_COLOR_TEX_3;
			current_vertex_size_ = sizeof(vertex_3_pos_normal_color_tex);
			return current_vertex_size_ * 3 * tri_total_count;
		}
		else if (have_pos && have_normal && have_texcoord)
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
		
		std::vector<Src*> input_srcs;
		Src* src;
		Input* input;
		
		int vertex_num, vertex_stride;
		int buffer_offset;
		int src_idx;
		
		// TODO: now only support 1 color input, and use it as vertex color
		int color_input_count;
		
		for (int tri_idx = 0; tri_idx < current_load_mesh_->triangles_array.size(); ++tri_idx)
		{
			Triangles* tri = current_load_mesh_->triangles_array[tri_idx];
			int input_num = tri->inputs.size();
			
			input_srcs.clear();
			for (int i = 0; i < input_num; ++i)
			{
				if (tri->inputs[i]->semantic == VERTEX)
				{
					input = current_load_mesh_->vertices_map[tri->inputs[i]->src];
					src = current_load_mesh_->src_map[input->src];
				}
				else
				{
					src = current_load_mesh_->src_map[tri->inputs[i]->src];
				}
				input_srcs.push_back(src);
			}
			
			vertex_num = tri->count * 3;
			vertex_stride = tri->primitives.size() / vertex_num;
			
			ASSERT((vertex_stride * vertex_num) == tri->primitives.size());
			
			for (int i = 0; i < vertex_num; ++i)
			{
				buffer_offset = 0;
				color_input_count = 0;
				
				// TODO: vertex data order need to match collada input order, should change implement way
				
				for (int j = 0; j < input_num; ++j)
				{
					src_idx = tri->primitives[i * vertex_stride + tri->inputs[j]->offset];
										
					FillSingleVertexInput(tri->inputs[j],
																input_srcs[j],
																src_idx,
																buffer_data,
																buffer_offset,
																color_input_count);
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
	
	SharedSkeleton* ColladaLoader::CreateSharedSkeleton(const std::vector<int>* anim_sections /*= NULL*/)
	{
		if (skeleton_map_.empty())
			return NULL;
		
		Skeleton* skeleton = NULL;
		
		// TODO: wrong usage, find out correct one
		
		int skel_idx = 0;
		int max_node = 0;
		std::map<std::string, Skeleton*>::iterator skel_it;
		for (skel_it = skeleton_map_.begin(); skel_it != skeleton_map_.end(); ++skel_it, ++skel_idx)
		{
			Skeleton* current_skel = skel_it->second;
			
			printf("skel[%d]: %d node, %d skin\n", skel_idx,
						 static_cast<int>(current_skel->nodes.size()),
						 static_cast<int>(current_skel->skin_refs.size()));
			
			for (int i = 0; i < current_skel->nodes.size(); ++i)
			{
				printf("  node[%d]: %s, parent %d\n", i,
							 current_skel->nodes[i]->name.c_str(),
							 current_skel->nodes[i]->parent_idx);
			}
			
			if (current_skel->nodes.size() > max_node)
			{
				max_node = current_skel->nodes.size();
				skeleton = current_skel;
			}
		}
		
		if (!skeleton)
			return NULL;
		
		//
		
		SharedSkeleton* share_skeleton = new SharedSkeleton;
		share_skeleton->skeleton_ref = new ERI::Skeleton;
		
		int node_num = skeleton->nodes.size();
		for (int i = 0; i < node_num; ++i)
		{
			ERI::SkeletonNode* skeleton_node = new ERI::SkeletonNode;
			skeleton_node->name = skeleton->nodes[i]->name;
			skeleton_node->parent_idx = skeleton->nodes[i]->parent_idx;
			skeleton_node->local_transform = skeleton->nodes[i]->local_transform;
			
			if (!skeleton->nodes[i]->joint_name.empty())
			{
				// find joint

				Joint* joint = NULL;
				for (int j = 0; j < skeleton->skin_refs.size(); ++j)
				{
					for (int k = 0; k < skeleton->skin_refs[j]->joint_array.size(); ++k)
					{
						if (skeleton->skin_refs[j]->joint_array[k]->name.compare(skeleton->nodes[i]->joint_name) == 0)
						{
							joint = skeleton->skin_refs[j]->joint_array[k];
							break;
						}
					}
					
					if (joint)
						break;
				}
				
				ASSERT(joint);
				
				//
				
				skeleton_node->inverse_bind_pose = joint->inverse_bind_pose_matrix;
				skeleton_node->is_joint = true;
			}
			
			share_skeleton->skeleton_ref->nodes.push_back(skeleton_node);
		}
		
		std::vector<int> joint_node_mapping;

		int skin_num = skeleton->skin_refs.size();
		for (int i = 0; i < skin_num; ++i)
		{
			Skin* skin = skeleton->skin_refs[i];
			current_load_mesh_ = skin->mesh_ref;
			GetVertexBufferSize();
			
			// setup joint -> node idx mapping
			
			joint_node_mapping.clear();
			int joint_num = skin->joint_array.size();
			for (int i = 0; i < joint_num; ++i)
			{
				for (int j = 0; j < node_num; ++j)
				{
					if (skin->joint_array[i]->name.compare(skeleton->nodes[j]->joint_name) == 0)
					{
						joint_node_mapping.push_back(j);
						break;
					}
				}
			}
			
			ASSERT(joint_node_mapping.size() == joint_num);
			
			//
			
			ERI::Mesh* mesh = CreateMesh(skin, joint_node_mapping);
			
			ASSERT(mesh);
			
			share_skeleton->mesh_refs.push_back(mesh);
		}

		AnimClip* anim;
		int anim_num = animation_array_.size();
		for (int i = 0; i < anim_num; ++i)
		{
			if (anim_sections)
			{
				int start_idx = 0;
				int section_frame_num;
				int section_num = anim_sections->size();
				for (int j = 0; j < section_num; ++j)
				{
					section_frame_num = (*anim_sections)[j];
					anim = CreateAnimClip(animation_array_[i], skeleton->nodes, start_idx, section_frame_num);
					
					ASSERT(anim);
					
					share_skeleton->anim_refs.push_back(anim);
					
					start_idx += section_frame_num;
				}
			}
			else
			{
				AnimClip* anim = CreateAnimClip(animation_array_[i], skeleton->nodes);
				
				ASSERT(anim);
				
				share_skeleton->anim_refs.push_back(anim);
			}
		}
		
		return share_skeleton;
	}
	
	void ColladaLoader::FillSingleVertexInput(const Input* input,
																						const Src* src,
																						int src_idx,
																						unsigned char* buffer_data,
																						int& buffer_offset,
																						int& color_input_count)
	{
		float color_values[4];
		bool is_color_for_alpha;
		float invert_v;
		
		if (input->semantic == COLOR)
		{
			++color_input_count;
			if (color_input_count > 1)
			{
				return;
			}
			else
			{
				ASSERT(src->stride <= 4);
				
				is_color_for_alpha = true;
				for (int k = 0; k < src->stride; ++k)
				{
					memcpy(&color_values[k],
								 &src->datas[src_idx * src->stride + k], 4);
					
					if (k > 0 && color_values[k] != color_values[k - 1])
						is_color_for_alpha = false;
				}
				
				unsigned char* color_buff = &buffer_data[buffer_offset];
				if (is_color_for_alpha)
				{
					color_buff[0] = color_buff[1] = color_buff[2] = 255;
					color_buff[3] = static_cast<unsigned char>(color_values[0] * 255);
				}
				else
				{
					for (int k = 0; k < src->stride; ++k)
					{
						color_buff[k] = static_cast<unsigned char>(color_values[k] * 255);
					}
					if (src->stride < 4)
						color_buff[3] = 255;
				}
				
				buffer_offset += 4;
				
				//// float color
				//float* color_buff = (float*)&buffer_data[buffer_offset];
				//color_buff[0] = color_buff[1] = color_buff[2] = 1.0f;
				//color_buff[3] =*(float*)(&src->datas[src_idx * src->stride]);
				//buffer_offset += 16;
			}
		}
		else if (input->semantic == TEXCOORD)
		{
			// TODO: check this, some resource's TEXCOORD stride will greater than 2 (S,T,P), we can only use 2
			
			memcpy(&buffer_data[buffer_offset],
						 &src->datas[src_idx * src->stride], 4);
			
			// TODO: check this, collada export's v is invert in our renderer
			invert_v = 1.0f - src->datas[src_idx * src->stride + 1];
			memcpy(&buffer_data[buffer_offset + 4], &invert_v, 4);
			
			buffer_offset += 8;
		}
		else
		{
			for (int k = 0; k < src->stride; ++k)
			{
				memcpy(&buffer_data[buffer_offset + k * 4],
							 &src->datas[src_idx * src->stride + k], 4);
				
				ASSERT(reinterpret_cast<long>(&buffer_data[buffer_offset + k * 4]) < reinterpret_cast<long>(buffer_data) + current_vertex_size_);
			}
			
			buffer_offset += src->stride * 4;
		}
	}
	
	ERI::Mesh* ColladaLoader::CreateMesh(Skin* skin, const std::vector<int>& joint_node_mapping)
	{
		ERI::Mesh* mesh = new ERI::Mesh;
		mesh->vertex_format = current_vertex_format_;
		mesh->vertex_size = current_vertex_size_;
		
		Triangles* tri;
		int input_num;
		std::vector<Src*> input_srcs;
		Src* src;
		Input* input;
		
		int vertex_num, vertex_stride;
		unsigned char* buffer_data;
		int buffer_offset;
		int src_idx;
		
		// TODO: now only support 1 color input, and use it as vertex color
		int color_input_count;
		
		ERI::Vertex* vertex;
		
		for (int tri_idx = 0; tri_idx < current_load_mesh_->triangles_array.size(); ++tri_idx)
		{
			tri = current_load_mesh_->triangles_array[tri_idx];
			input_num = tri->inputs.size();
			
			input_srcs.clear();
			for (int i = 0; i < input_num; ++i)
			{
				if (tri->inputs[i]->semantic == VERTEX)
				{
					input = current_load_mesh_->vertices_map[tri->inputs[i]->src];
					src = current_load_mesh_->src_map[input->src];
				}
				else
				{
					src = current_load_mesh_->src_map[tri->inputs[i]->src];
				}
				input_srcs.push_back(src);
			}
			
			vertex_num = tri->count * 3;
			vertex_stride = tri->primitives.size() / vertex_num;
			
			ASSERT((vertex_stride * vertex_num) == tri->primitives.size());
			
			for (int i = 0; i < vertex_num; ++i)
			{
				vertex = new Vertex;
				vertex->data = malloc(current_vertex_size_);
				
				buffer_data = static_cast<unsigned char*>(vertex->data);
				buffer_offset = 0;
				color_input_count = 0;
				
				// TODO: vertex data order need to match collada input order, should change implement way
				
				for (int j = 0; j < input_num; ++j)
				{
					src_idx = tri->primitives[i * vertex_stride + tri->inputs[j]->offset];
					
					FillSingleVertexInput(tri->inputs[j],
																input_srcs[j],
																src_idx,
																buffer_data,
																buffer_offset,
																color_input_count);

					// skin vertex weight
					
					if (skin && tri->inputs[j]->semantic == VERTEX)
					{
						ASSERT(src_idx < skin->vertex_weight_array.size());
						
						VertexWeight* vertex_weight = skin->vertex_weight_array[src_idx];
						
						for (int m = 0; m < vertex_weight->joints.size(); ++m)
						{
							vertex->influence_nodes.push_back(joint_node_mapping[vertex_weight->joints[m]]);
							vertex->influence_weights.push_back(vertex_weight->weights[m]);
						}
					}
				}
				
				mesh->vertices.push_back(vertex);
			}
		}
		
		return mesh;
	}
	
	ERI::AnimClip* ColladaLoader::CreateAnimClip(Animation* anim,
												 const std::vector<SkeletonNode*>& nodes,
												 int start_frame /*= -1*/,
												 int frame_count /*= -1*/)
	{
		ASSERT(anim);
		
		AnimClip* anim_clip = new AnimClip;
		PoseSample* pose_sample;
		
		std::string target_name;
		
		int channel_num = anim->channel_array.size();
		for (int i = 0; i < channel_num; ++i)
		{
			Sampler* sampler = anim->sampler_map[anim->channel_array[i]->sampler_id];
			// already check exist when parsing
		
			ASSERT(sampler);
			
			pose_sample = new PoseSample;
			
			// find corresponding node idx
			
			target_name = anim->channel_array[i]->target;
			size_t pos = target_name.find('/');
			if (pos != std::string::npos)
				target_name = target_name.substr(0, pos);
			
			int node_num = nodes.size();
			for (int j = 0; j < node_num; ++j)
			{
				if (target_name.compare(nodes[j]->name) == 0)
				{
					pose_sample->skeleton_node_idx = j;
					break;
				}
			}
			
			ASSERT(pose_sample->skeleton_node_idx != -1);
			
			//
			
			int key_num = sampler->times.size();
			int start_key = 0;
			float time_offset = 0;
			
			if (start_frame != -1 && frame_count != -1)
			{
				ASSERT(start_frame >= 0 && start_frame < key_num && frame_count > 0 && (start_frame + frame_count) <= key_num);
				
				key_num = frame_count;
				start_key = start_frame;
				if (start_key > 0)
				{
					time_offset = -sampler->times[start_key - 1];
				}
			}
			
			pose_sample->transforms.resize(key_num);
			
			for (int j = start_key; j < (start_key + key_num); ++j)
			{
				pose_sample->times.push_back(sampler->times[j] + time_offset);
				
				sampler->transforms[j].ExtractTransform(pose_sample->transforms[j - start_key].scale,
														pose_sample->transforms[j - start_key].rotate,
														pose_sample->transforms[j - start_key].translate);
			}
			
			//
			
			anim_clip->pose_samples.push_back(pose_sample);
		}
		
		return anim_clip;
	}
	
	void ColladaLoader::ParseLibraryGeometries(rapidxml::xml_node<>* node)
	{
		std::string geometry_id;
		Mesh* geometry = NULL;
		
		xml_node<>* node2 = node->first_node();
		xml_node<>* node3;
		xml_node<>* node4;
		std::string s;
		
		while (node2)
		{
			if (strcmp(node2->name(), "geometry") == 0)
			{
				if (GetAttrStr(node2, "id", s))
				{
					if (geometry)
					{
						geometry_map_[geometry_id] = geometry;
						current_load_mesh_ = geometry;
					}
					
					geometry = new Mesh;
					geometry_id = s;
					
					ASSERT(geometry_map_.find(geometry_id) == geometry_map_.end());
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
					Src* src = ParseSrc(node3, s);
					if (src)
					{
						ASSERT(geometry->src_map.find(s) == geometry->src_map.end());
						
						geometry->src_map[s] = src;
					}
				}
				else if (strcmp(node3->name(), "vertices") == 0)
				{
					if (!GetAttrStr(node3, "id", s))
					{
						node3 = node3->next_sibling();
						continue;
					}
					
					node4 = node3->first_node("input");
					if (!node4)
					{
						node3 = node3->next_sibling();
						continue;
					}
					
					Input* input = ParseInput(node4, false);
					if (input)
					{
						ASSERT(geometry->vertices_map.find(s) == geometry->vertices_map.end());

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
							Input* input = ParseInput(node4, true);
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
						node3 = node3->next_sibling();
						continue;
					}
					
					geometry->triangles_array.push_back(tris);
				}
				
				node3 = node3->next_sibling();
			}
			
			node2 = node2->next_sibling();
		}
		
		if (geometry)
		{
			geometry_map_[geometry_id] = geometry;
			current_load_mesh_ = geometry;
		}
	}
	
	void ColladaLoader::ParseLibraryControllers(rapidxml::xml_node<>* node)
	{
		std::string skin_id;
		Skin* skin = NULL;
		
		xml_node<>* node2 = node->first_node();
		xml_node<>* node3;
		xml_node<>* node4;
		std::string s;
		
		while (node2)
		{
			if (strcmp(node2->name(), "controller") == 0)
			{
				if (GetAttrStr(node2, "id", s))
				{
					if (skin)
					{
						skin_map_[skin_id] = skin;
					}
					
					skin = new Skin;
					skin_id = s;
					
					ASSERT(skin_map_.find(skin_id) == skin_map_.end());
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
			
			// current support skin only
			node3 = node2->first_node("skin");
			if (!node3)
			{
				node2 = node2->next_sibling();
				continue;
			}

			if (GetAttrStr(node3, "source", s))
			{
				if (s[0] == '#')
					s = s.substr(1);
			}
			else
			{
				node2 = node2->next_sibling();
				continue;
			}
			
			std::map<std::string, Mesh*>::iterator it = geometry_map_.find(s);
			
			ASSERT(it != geometry_map_.end());
			
			skin->mesh_ref = it->second;
			
			node3 = node3->first_node();
			while (node3)
			{
				if (strcmp(node3->name(), "bind_shape_matrix") == 0)
				{
					s = node3->value();
					std::istringstream iss(s);
					float value;
					int idx = 0;
					while (!iss.eof())
					{
						iss >> value;
						if (Abs(value) < Math::ZERO_TOLERANCE) value = 0.0f;
						skin->bind_shape_matrix.Set(idx / 4, idx % 4, value);
						++idx;
					}
				}
				else if (strcmp(node3->name(), "source") == 0)
				{
					Src* src = ParseSrc(node3, s);
					if (src)
					{
						ASSERT(skin->src_map.find(s) == skin->src_map.end());
						
						skin->src_map[s] = src;
					}
				}
				else if (strcmp(node3->name(), "joints") == 0)
				{
					Input* input = NULL;
					
					node4 = node3->first_node("input");
					while (node4)
					{
						input = ParseInput(node4, false);
						if (input)
						{
							if (input->semantic == JOINT)
							{
								std::map<std::string, Src*>::iterator it = skin->src_map.find(input->src);
								
								ASSERT(it != skin->src_map.end());
								
								Src* src = it->second;
								
								ASSERT(src->count == src->str_datas.size());
								ASSERT(src->stride == 1);
									
								for (int i = 0; i < src->count; ++i)
								{
									Joint* joint = new Joint;
									joint->name = src->str_datas[i];
									skin->joint_array.push_back(joint);
								}
							}
							else if (input->semantic == INV_BIND_MATRIX)
							{
								std::map<std::string, Src*>::iterator it = skin->src_map.find(input->src);
								
								ASSERT(it != skin->src_map.end());
								
								Src* src = it->second;
								
								ASSERT(src->count == skin->joint_array.size());
								ASSERT(src->count == (src->datas.size() / src->stride));
								ASSERT(src->stride == 16);

								for (int i = 0; i < src->count; ++i)
								{
									for (int j = 0; j < src->stride; ++j)
									{
										skin->joint_array[i]->inverse_bind_pose_matrix.Set(j / 4, j % 4, src->datas[i * src->stride + j]);
									}
								}
							}
							
							delete input;
						}
						
						node4 = node4->next_sibling("input");
					}
				}
				else if (strcmp(node3->name(), "vertex_weights") == 0)
				{
					std::vector<int> joint_match_idxs;
					std::vector<int> vertex_influence_joint_counts;
					Src* weight_src = NULL;
					
					node4 = node3->first_node();
					while (node4)
					{
						if (strcmp(node4->name(), "input") == 0)
						{
							Input* input = ParseInput(node4, true);
							if (input)
							{
								if (input->semantic == JOINT)
								{
									ASSERT(input->offset == 0);
									
									std::map<std::string, Src*>::iterator it = skin->src_map.find(input->src);
									
									ASSERT(it != skin->src_map.end());
									
									Src* src = it->second;
									
									ASSERT(src->count == src->str_datas.size());
									ASSERT(src->stride == 1);
								
									for (int i = 0; i < src->count; ++i)
									{
										joint_match_idxs.push_back(FindMatchJointIdx(skin, src->str_datas[i]));
									}
								}
								else if (input->semantic == WEIGHT)
								{
									ASSERT(input->offset == 1);
									
									std::map<std::string, Src*>::iterator it = skin->src_map.find(input->src);
									
									ASSERT(it != skin->src_map.end());
									
									weight_src = it->second;
								}
								
								delete input;
							}
						}
						else if (strcmp(node4->name(), "vcount") == 0)
						{
							s = node4->value();
							std::istringstream iss(s);
							int value;
							while (!iss.eof())
							{
								iss >> value;
								vertex_influence_joint_counts.push_back(value);
							}
						}
						else if (strcmp(node4->name(), "v") == 0)
						{
							s = node4->value();
							std::istringstream iss(s);
							
							int joint_idx;
							int weight_idx;
							float weight, total_weight;
							int vertex_num = vertex_influence_joint_counts.size();
							for (int i = 0; i < vertex_num; ++i)
							{
								VertexWeight* vertex_weight = new VertexWeight;
								total_weight = 0.0f;
								
								for (int j = 0; j < vertex_influence_joint_counts[i]; ++j)
								{
									ASSERT(!iss.eof());
									
									iss >> joint_idx;
									
									ASSERT(joint_idx < joint_match_idxs.size());
									
									if (joint_idx >= 0)
										joint_idx = joint_match_idxs[joint_idx];
									
									vertex_weight->joints.push_back(joint_idx);
									
									ASSERT(!iss.eof());
									
									iss >> weight_idx;
									
									ASSERT(weight_idx >= 0 && weight_src && weight_idx < weight_src->datas.size());
									
									weight = weight_src->datas[weight_idx];
									
									if (weight > 0)
									{
										total_weight += weight;
										vertex_weight->weights.push_back(weight);
									}
									else
									{
										vertex_weight->joints.pop_back();
									}
								}
								
								if (total_weight > 0.0f)
								{
									for (int j = 0; j < vertex_weight->joints.size(); ++j)
									{
										vertex_weight->weights[j] /= total_weight;
									}
								}
								
								skin->vertex_weight_array.push_back(vertex_weight);
							}
						}
						
						node4 = node4->next_sibling();
					}
				}
				
				node3 = node3->next_sibling();
			}
			
			node2 = node2->next_sibling();
		}
		
		if (skin)
		{
			skin_map_[skin_id] = skin;
		}
	}
	
	void ColladaLoader::ParseLibraryVisualScenes(rapidxml::xml_node<>* node)
	{
		node = node->first_node("visual_scene");
		if (!node)
			return;
		
		xml_node<>* root_node = node;
		
		xml_node<>* node2;
		xml_node<>* node3;
		std::string s;
		
		Skeleton* skeletion;
		Skin* skin;
		
		node = root_node->first_node("node");
		while (node)
		{
			node2 = FindNode(node, "instance_controller");
			if (node2)
			{
				if (GetAttrStr(node2, "url", s))
				{
					if (s[0] == '#')
						s = s.substr(1);
					
					std::map<std::string, Skin*>::iterator skin_it = skin_map_.find(s);
					
					ASSERT(skin_it != skin_map_.end());
					
					skin = skin_it->second;
					
					node3 = node2->first_node("skeleton");
					while (node3)
					{
						s = node3->value();
						if (!s.empty())
						{
							if (s[0] == '#')
								s = s.substr(1);
							
							std::map<std::string, Skeleton*>::iterator skel_it = skeleton_map_.find(s);
							
							if (skel_it == skeleton_map_.end())
							{
								skeletion = new Skeleton;
								skeleton_map_[s] = skeletion;
							}
							else
							{
								skeletion = skel_it->second;
							}

							skeletion->skin_refs.push_back(skin);
						}
						
						node3 = node3->next_sibling("skeleton");
					}
				}
			}
			
			node = node->next_sibling("node");
		}
		
		std::map<std::string, Skeleton*>::iterator skel_it;
		node = root_node->first_node("node");
		while (node)
		{
			if (GetAttrStr(node, "id", s))
			{
				skel_it = skeleton_map_.find(s);
				if (skel_it != skeleton_map_.end())
				{
					skeletion = skel_it->second;
					ParseSkeletonNode(node, skeletion, -1);
				}
				else if(FindSkeletonNode(node->first_node("node"), skeletion))
				{
					ParseSkeletonNode(node, skeletion, -1);
				}
			}
			
			node = node->next_sibling("node");
		}
	}
	
	void ColladaLoader::ParseLibraryAnimations(rapidxml::xml_node<>* node)
	{
		Animation* anim = NULL;
		
		xml_node<>* node2;
		xml_node<>* node3;
		std::string s;

		node = node->first_node();
		while (node)
		{
			if (strcmp(node->name(), "animation") == 0)
			{
				if (anim)
				{
					animation_array_.push_back(anim);
				}
					
				anim = new Animation;
			}
			else
			{
				node = node->next_sibling();
				continue;
			}
			
			node2 = node->first_node();
			while (node2)
			{
				if (strcmp(node2->name(), "source") == 0)
				{
					Src* src = ParseSrc(node2, s);
					if (src)
					{
						ASSERT(anim->src_map.find(s) == anim->src_map.end());
						
						anim->src_map[s] = src;
					}
				}
				else if (strcmp(node2->name(), "sampler") == 0)
				{
					Sampler* sampler = new Sampler;
					std::string sampler_id;
					
					GetAttrStr(node2, "id", sampler_id);
					
					ASSERT(!sampler_id.empty());
					ASSERT(anim->sampler_map.find(sampler_id) == anim->sampler_map.end());
					
					node3 = node2->first_node("input");
					while (node3)
					{
						Input* input = ParseInput(node3, false);
						if (input)
						{
							if (input->semantic == INPUT)
							{
								std::map<std::string, Src*>::iterator it = anim->src_map.find(input->src);
								
								ASSERT(it != anim->src_map.end());
								
								Src* src = it->second;
								
								ASSERT(src->count == src->datas.size());
								ASSERT(src->stride == 1);
								
								for (int i = 0; i < src->count; ++i)
								{
									sampler->times.push_back(src->datas[i]);
								}
							}
							else if (input->semantic == OUTPUT)
							{
								std::map<std::string, Src*>::iterator it = anim->src_map.find(input->src);
								
								ASSERT(it != anim->src_map.end());
								
								Src* src = it->second;
								
								ASSERT(src->count == sampler->times.size());
								ASSERT(src->count == (src->datas.size() / src->stride));
								ASSERT(src->stride == 16);
								
								sampler->transforms.resize(src->count);
								
								for (int i = 0; i < src->count; ++i)
								{
									for (int j = 0; j < src->stride; ++j)
									{
										sampler->transforms[i].Set(j / 4, j % 4, src->datas[i * src->stride + j]);
									}
								}
							}
							else if (input->semantic == INTERPOLATION)
							{
								std::map<std::string, Src*>::iterator it = anim->src_map.find(input->src);
								
								ASSERT(it != anim->src_map.end());
								
								Src* src = it->second;
								
								ASSERT(src->count == sampler->times.size());
								ASSERT(src->count == src->str_datas.size());
								ASSERT(src->stride == 1);
								
								for (int i = 0; i < src->count; ++i)
								{
									if (src->str_datas[i].compare("LINEAR") == 0)
									{
										sampler->interpolations.push_back(LINEAR);
									}
									else
									{
										sampler->interpolations.push_back(NOT_SUPPORT);
									}
								}
							}
							
							delete input;
						}
						
						node3 = node3->next_sibling("input");
					}
					
					if (!sampler->times.empty() && !sampler->transforms.empty())
					{
						anim->sampler_map[sampler_id] = sampler;
					}
				}
				else if (strcmp(node2->name(), "channel") == 0)
				{
					GetAttrStr(node2, "source", s);
					
					if (!s.empty() && s[0] == '#')
						s = s.substr(1);
					
					ASSERT(anim->sampler_map.find(s) != anim->sampler_map.end());
					
					Channel* channel = new Channel;
					channel->sampler_id = s;
					
					GetAttrStr(node2, "target", channel->target);
					
					anim->channel_array.push_back(channel);
				}
				
				node2 = node2->next_sibling();
			}
			
			node = node->next_sibling();
		}
		
		if (anim)
		{
			animation_array_.push_back(anim);
		}
	}
	
	ColladaLoader::Src* ColladaLoader::ParseSrc(rapidxml::xml_node<>* node, std::string& out_id)
	{
		std::string s;
		
		if (!GetAttrStr(node, "id", out_id))
			return NULL;

		bool is_float = true;
		
		xml_node<>* node2 = node->first_node("float_array");
		if (!node2)
		{
			node2 = node->first_node("Name_array");
			if (!node2)
				return NULL;
			
			is_float = false;
		}
		
		s = node2->value();
		std::istringstream iss(s);
		
		Src* src = new Src;
		
		int data_num = 0;
		
		if (is_float)
		{
			float value;
			while (!iss.eof())
			{
				iss >> value;
				src->datas.push_back(value);
			}
			
			data_num = src->datas.size();
		}
		else
		{
			std::string value;
			while (!iss.eof())
			{
				iss >> value;
				src->str_datas.push_back(value);
			}
			
			data_num = src->str_datas.size();
		}

		
		node2 = node->first_node("technique_common");
		if (!node2)
		{
			delete src;
			return NULL;
		}
		
		node2 = node2->first_node("accessor");
		if (!node2)
		{
			delete src;
			return NULL;
		}
		
		if (!GetAttrInt(node2, "count", src->count))
		{
			delete src;
			return NULL;
		}
		
		if (!GetAttrInt(node2, "stride", src->stride))
		{
			delete src;
			return NULL;
		}
		
		if (src->count * src->stride != data_num)
		{
			delete src;
			return NULL;
		}
		
		return src;
	}

	ColladaLoader::Input* ColladaLoader::ParseInput(rapidxml::xml_node<>* node, bool is_need_offset)
	{
		std::string s;
		
		if (!GetAttrStr(node, "semantic", s))
			return NULL;
		
		Input* input = new Input;
		
		bool is_need_set = false;
		
		if (s.compare("POSITION") == 0)
		{
			input->semantic = POSITION;
		}
		else if (s.compare("VERTEX") == 0)
		{
			input->semantic = VERTEX;
		}
		else if (s.compare("NORMAL") == 0)
		{
			input->semantic = NORMAL;
		}
		else if (s.compare("COLOR") == 0)
		{
			input->semantic = COLOR;
		}
		else if (s.compare("TEXCOORD") == 0)
		{
			input->semantic = TEXCOORD;
			is_need_set = true;
		}
		else if (s.compare("JOINT") == 0)
		{
			input->semantic = JOINT;
		}
		else if (s.compare("INV_BIND_MATRIX") == 0)
		{
			input->semantic = INV_BIND_MATRIX;
		}
		else if (s.compare("WEIGHT") == 0)
		{
			input->semantic = WEIGHT;
		}
		else if (s.compare("INPUT") == 0)
		{
			input->semantic = INPUT;
		}
		else if (s.compare("OUTPUT") == 0)
		{
			input->semantic = OUTPUT;
		}
		else if (s.compare("INTERPOLATION") == 0)
		{
			input->semantic = INTERPOLATION;
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
	
	void ColladaLoader::ParseSkeletonNode(rapidxml::xml_node<>* node, Skeleton* skeleton, int parent_idx)
	{
		std::string s;
		
		if (!GetAttrStr(node, "id", s) || s.empty())
			return;

		SkeletonNode* skeleton_node = new SkeletonNode;
		skeleton_node->name = s;
		skeleton_node->parent_idx = parent_idx;
		
		if (GetAttrStr(node, "type", s) && s.compare("JOINT") == 0)
		{
			GetAttrStr(node, "sid", skeleton_node->joint_name);
		}
		
		skeleton->nodes.push_back(skeleton_node);
		
		int self_idx = skeleton->nodes.size() - 1;
		
		node = node->first_node();
		while (node)
		{
			if (strcmp(node->name(), "matrix") == 0)
			{
				s = node->value();
				std::istringstream iss(s);
				float value;
				int idx = 0;
				while (!iss.eof())
				{
					iss >> value;
					if (Abs(value) < Math::ZERO_TOLERANCE) value = 0.0f;
					skeleton_node->local_transform.Set(idx / 4, idx % 4, value);
					++idx;
				}
			}
			else if (strcmp(node->name(), "node") == 0)
			{
				ParseSkeletonNode(node, skeleton, self_idx);
			}
			
			node = node->next_sibling();
		}
	}
	
	bool ColladaLoader::FindSkeletonNode(rapidxml::xml_node<>* node, Skeleton*& out_skeleton)
	{
		if (!node)
			return false;
		
		std::string s;
		std::map<std::string, Skeleton*>::iterator skel_it;
		
		do
		{
			if (GetAttrStr(node, "id", s))
			{
				skel_it = skeleton_map_.find(s);
				if (skel_it != skeleton_map_.end())
				{
					out_skeleton = skel_it->second;
					return true;
				}
				else
				{
					if (FindSkeletonNode(node->first_node("node"), out_skeleton))
						return true;
				}
			}
			
			node = node->next_sibling("node");
		}
		while (node);
		
		return false;
	}
	
	int ColladaLoader::FindMatchJointIdx(Skin* skin, const std::string& joint_name)
	{
		ASSERT(skin);
		
		int num = skin->joint_array.size();
		for (int i = 0; i < num; ++i)
		{
			if (skin->joint_array[i]->name.compare(joint_name) == 0)
				return i;
		}
		
		return -1;
	}
	
	rapidxml::xml_node<>* ColladaLoader::FindNode(rapidxml::xml_node<>* node, const char* name)
	{
		if (!node)
			return NULL;
		
		if (strcmp(node->name(), name) == 0)
			return node;
		
		rapidxml::xml_node<>* node2;
		node = node->first_node();
		while (node)
		{
			node2 = FindNode(node, name);
			if (node2)
				return node2;
			
			node = node->next_sibling();
		}
		
		return NULL;
	}

}
