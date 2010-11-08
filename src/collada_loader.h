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
#include "math_helper.h"

namespace ERI
{
	class SharedSkeleton;
	class Mesh;
	class AnimClip;
	
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
		
		//
		
		ERI::SharedSkeleton* CreateSharedSkeleton();

	private:
		enum Semantic
		{
			VERTEX,
			POSITION,
			NORMAL,
			TEXCOORD,
			JOINT,
			INV_BIND_MATRIX,
			WEIGHT,
			INPUT,
			OUTPUT,
			INTERPOLATION
		};
		
		struct Src
		{
			std::vector<float>			datas;
			std::vector<std::string>	str_datas;
			int							count;
			int							stride;
		};
		
		struct Input
		{
			Semantic		semantic;
			std::string		src;
			int				offset;
			int				set;
		};
		
		//
		
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
		
		//
		
		struct Joint
		{
			std::string		name;
			Matrix4			inverse_bind_pose_matrix;
		};
		
		struct VertexWeight
		{
			std::vector<int>		joints;
			std::vector<float>		weights;
		};
		
		struct Skin
		{
			~Skin();
			
			Mesh*							mesh_ref;
			Matrix4							bind_shape_matrix;
			std::map<std::string, Src*>		src_map;
			std::vector<Joint*>				joint_array;
			std::vector<VertexWeight*>		vertex_weight_array;
		};
		
		//
		
		struct SkeletonNode
		{
			std::string		name;
			int				parent_idx;
			Matrix4			local_transform;
			std::string		joint_name;
		};
		
		struct Skeleton
		{
			~Skeleton();
			
			std::vector<SkeletonNode*>	nodes;
			std::vector<Skin*>			skin_refs;
		};
		
		//
		
		enum InterpolationType
		{
			LINEAR,
			NOT_SUPPORT
		};
		
		struct Sampler
		{
			std::vector<float>				times;
			std::vector<Matrix4>			transforms;
			std::vector<InterpolationType>	interpolations;
		};
		
		struct Channel
		{
			std::string		sampler_id;
			std::string		target;
		};
		
		struct Animation
		{
			~Animation();
			
			std::map<std::string, Src*>		src_map;
			std::map<std::string, Sampler*>	sampler_map;
			std::vector<Channel*>			channel_array;
		};
		
		//
		
		ERI::Mesh* CreateMesh(Skin* skin, const std::vector<int>& joint_node_mapping);
		ERI::AnimClip* CreateAnimClip(Animation* anim, const std::vector<SkeletonNode*> nodes);
		
		//
		
		void ParseLibraryGeometries(rapidxml::xml_node<>* node);
		void ParseLibraryControllers(rapidxml::xml_node<>* node);
		void ParseLibraryVisualScenes(rapidxml::xml_node<>* node);
		void ParseLibraryAnimations(rapidxml::xml_node<>* node);
		
		Src* ParseSrc(rapidxml::xml_node<>* node, std::string& out_id);
		Input* ParseInput(rapidxml::xml_node<>* node, bool is_need_offset);
		
		void ParseSkeletonNode(rapidxml::xml_node<>* node, Skeleton* skeleton, int parent_idx);
		
		int FindMatchJointIdx(Skin* skin, const std::string& joint_name);
		
		//
		
		std::map<std::string, Mesh*>	geometry_map_;
		
		Mesh*							current_load_mesh_;
		GLenum							current_vertex_type_;
		VertexFormat					current_vertex_format_;
		int								current_vertex_size_;
		
		std::map<std::string, Skin*>	skin_map_;
		
		//
		
		std::map<std::string, Skeleton*>	skeleton_map_;
		
		//
		
		std::vector<Animation*>			animation_array_;
	};

}

#endif // ERI_COLLADA_LOADER
