//
//  shared_skeleton.cpp
//  dae2skelmesh
//
//  Created by chiayu on 2011/4/6.
//  Copyright 2011年 __MyCompanyName__. All rights reserved.
//

#include "shared_skeleton.h"

#include <fstream>

namespace ERI
{
	
#pragma mark Skeleton

Skeleton::~Skeleton()
{
	for (int i = 0; i < nodes.size(); ++i)
	{
		delete nodes[i];
	}
}

#pragma mark Vertex

Vertex::~Vertex()
{
	free(data);
}

#pragma mark Mesh

Mesh::~Mesh()
{
	for (int i = 0; i < vertices.size(); ++i)
	{
		delete vertices[i];
	}
}

#pragma mark AnimClip

AnimClip::~AnimClip()
{
	for (int i = 0; i < pose_samples.size(); ++i)
	{
		delete pose_samples[i];
	}
}

#pragma mark SharedSkeleton

SharedSkeleton::~SharedSkeleton()
{
	// TODO: actually we only have reference so can't delete it
	
	if (bounding) delete bounding;
	
	delete skeleton_ref;
	
	for (int i = 0; i < mesh_refs.size(); ++i)
	{
		delete mesh_refs[i];
	}
	
	for (int i = 0; i < anim_refs.size(); ++i)
	{
		delete anim_refs[i];
	}
}

void SharedSkeleton::CalculateBounding()
{
	// TODO: should calculate by all mesh?
	
	if (bounding)
		return;
	
	bounding = new Sphere;
	
	Vector3 min_pos, max_pos, pos;
	
	Vertex* vertex;
	
	for (int k = 0; k < mesh_refs.size(); ++k)
	{
		const Mesh* mesh = mesh_refs[k];
		void* single_buffer = malloc(mesh->vertex_size);
		
		int vertex_num = mesh->vertices.size();
		for (int i = 0; i < vertex_num; ++i)
		{
			vertex = mesh->vertices[i];
			memcpy(single_buffer, vertex->data, mesh->vertex_size);
			
			float* float_value = static_cast<float*>(single_buffer);
			
			pos.x = float_value[0];
			pos.y = float_value[1];
			pos.z = float_value[2];
			
			if (k == 0 && i == 0)
			{
				min_pos = max_pos = pos;
			}
			else
			{
				if (pos.x < min_pos.x) min_pos.x = pos.x;
				if (pos.y < min_pos.y) min_pos.y = pos.y;
				if (pos.z < min_pos.z) min_pos.z = pos.z;
				if (pos.x > max_pos.x) max_pos.x = pos.x;
				if (pos.y > max_pos.y) max_pos.y = pos.y;
				if (pos.z > max_pos.z) max_pos.z = pos.z;
			}
		}
		
		free(single_buffer);
	}
	
	bounding->center = (min_pos + max_pos) * 0.5f;
	pos = max_pos - min_pos;
	bounding->radius = Max(pos.x, pos.y);
	bounding->radius = Max(bounding->radius, pos.z);
	bounding->radius *= 0.5f;
	
	printf("bounding sphere center(%.2f, %.2f, %.2f) radius %.2f\n", bounding->center.x, bounding->center.y, bounding->center.z, bounding->radius);
}
	
#pragma mark binary SharedSkeleton save/load function
	
void SaveSharedSkeletonToBinaryFile(const SharedSkeleton* skel, const std::string& path)
{
	ASSERT(skel);
	
	printf("start save file [%s] ...\n", path.c_str());
	
	std::ofstream ofs(path.c_str(), std::ios::out | std::ios::binary);
	
	bool have_skeleton = skel->skeleton_ref ? true : false;
	ofs.write(reinterpret_cast<char*>(&have_skeleton), sizeof(have_skeleton));
	if (skel->skeleton_ref)
	{
		int node_num = skel->skeleton_ref->nodes.size();
		ofs.write(reinterpret_cast<char*>(&node_num), sizeof(node_num));
		for (int i = 0; i < node_num; ++i)
		{
			SkeletonNode* node = skel->skeleton_ref->nodes[i];
			
			int length = node->name.length();
			ofs.write(reinterpret_cast<char*>(&length), sizeof(length));
			ofs.write(const_cast<char*>(node->name.c_str()), length);
			
			ofs.write(reinterpret_cast<char*>(&node->parent_idx), sizeof(node->parent_idx));
			
			ofs.write(reinterpret_cast<char*>(&node->local_transform), sizeof(node->local_transform));
			ofs.write(reinterpret_cast<char*>(&node->inverse_bind_pose), sizeof(node->inverse_bind_pose));
			
			ofs.write(reinterpret_cast<char*>(&node->is_joint), sizeof(node->is_joint));
		}
	}
	
	int mesh_num = skel->mesh_refs.size();
	ofs.write(reinterpret_cast<char*>(&mesh_num), sizeof(mesh_num));
	for (int i = 0; i < mesh_num; ++i)
	{
		Mesh* mesh = skel->mesh_refs[i];
		
		ofs.write(reinterpret_cast<char*>(&mesh->vertex_size), sizeof(mesh->vertex_size));
		
		int vertex_num = mesh->vertices.size();
		ofs.write(reinterpret_cast<char*>(&vertex_num), sizeof(vertex_num));
		for (int j = 0; j < vertex_num; ++j)
		{
			ofs.write(reinterpret_cast<char*>(mesh->vertices[j]->data), mesh->vertex_size);
			
			int influence_num = mesh->vertices[j]->influence_nodes.size();
			ofs.write(reinterpret_cast<char*>(&influence_num), sizeof(influence_num));
			for (int k = 0; k < influence_num; ++k)
			{
				ofs.write(reinterpret_cast<char*>(&mesh->vertices[j]->influence_nodes[k]), sizeof(mesh->vertices[j]->influence_nodes[k]));
				ofs.write(reinterpret_cast<char*>(&mesh->vertices[j]->influence_weights[k]), sizeof(mesh->vertices[j]->influence_weights[k]));
			}
		}
		
		int idx_num = mesh->indices.size();
		ofs.write(reinterpret_cast<char*>(&idx_num), sizeof(idx_num));
		for (int j = 0; j < idx_num; ++j)
		{
			ofs.write(reinterpret_cast<char*>(&mesh->indices[j]), sizeof(mesh->indices[j]));
		}
		
		ofs.write(reinterpret_cast<char*>(&mesh->vertex_format), sizeof(VertexFormat));
	}
	
	int anim_num = skel->anim_refs.size();
	ofs.write(reinterpret_cast<char*>(&anim_num), sizeof(anim_num));
	for (int i = 0; i < anim_num; ++i)
	{
		int pose_num = skel->anim_refs[i]->pose_samples.size();
		ofs.write(reinterpret_cast<char*>(&pose_num), sizeof(pose_num));
		for (int j = 0; j < pose_num; ++j)
		{
			PoseSample* pose = skel->anim_refs[i]->pose_samples[j];
			
			ofs.write(reinterpret_cast<char*>(&pose->skeleton_node_idx), sizeof(pose->skeleton_node_idx));
			
			int time_num = pose->times.size();
			ofs.write(reinterpret_cast<char*>(&time_num), sizeof(time_num));
			for (int k = 0; k < time_num; ++k)
				ofs.write(reinterpret_cast<char*>(&pose->times[k]), sizeof(pose->times[k]));
			
			int transform_num = pose->transforms.size();
			ofs.write(reinterpret_cast<char*>(&transform_num), sizeof(transform_num));
			for (int k = 0; k < transform_num; ++k)
				ofs.write(reinterpret_cast<char*>(&pose->transforms[k]), sizeof(Transform));
		}
	}
	
	bool have_bounding = skel->bounding ? true : false;
	ofs.write(reinterpret_cast<char*>(&have_bounding), sizeof(have_bounding));
	if (skel->bounding)
	{
		ofs.write(reinterpret_cast<char*>(skel->bounding), sizeof(Sphere));
	}
	
	ofs.close();
	
	printf("save end!\n");
}

SharedSkeleton* LoadSharedSkeletonFromBinaryFile(const std::string& path)
{
	SharedSkeleton* skel = new SharedSkeleton;
	
	std::ifstream ifs(path.c_str(), std::ios::in/* | std::ios::binary*/);
	
	bool have_skeleton;
	ifs.read(reinterpret_cast<char*>(&have_skeleton), sizeof(have_skeleton));
	if (have_skeleton)
	{
		skel->skeleton_ref = new Skeleton;
		
		int node_num;
		ifs.read(reinterpret_cast<char*>(&node_num), sizeof(node_num));
		for (int i = 0; i < node_num; ++i)
		{
			SkeletonNode* node = new SkeletonNode;
			
			int length;
			ifs.read(reinterpret_cast<char*>(&length), sizeof(length));
			char name[64];
			memset(name, 0, 64);
			ifs.read(name, length);
			node->name = name;
			
			ifs.read(reinterpret_cast<char*>(&node->parent_idx), sizeof(node->parent_idx));
			
			ifs.read(reinterpret_cast<char*>(&node->local_transform), sizeof(node->local_transform));
			ifs.read(reinterpret_cast<char*>(&node->inverse_bind_pose), sizeof(node->inverse_bind_pose));
			
			ifs.read(reinterpret_cast<char*>(&node->is_joint), sizeof(node->is_joint));
			
			skel->skeleton_ref->nodes.push_back(node);
		}
	}
	
	int mesh_num;
	ifs.read(reinterpret_cast<char*>(&mesh_num), sizeof(mesh_num));
	for (int i = 0; i < mesh_num; ++i)
	{
		Mesh* mesh = new Mesh;
		
		ifs.read(reinterpret_cast<char*>(&mesh->vertex_size), sizeof(mesh->vertex_size));
		
		int vertex_num;
		ifs.read(reinterpret_cast<char*>(&vertex_num), sizeof(vertex_num));
		for (int j = 0; j < vertex_num; ++j)
		{
			Vertex* vertex = new Vertex;
			vertex->data = malloc(mesh->vertex_size);
			
			ifs.read(reinterpret_cast<char*>(vertex->data), mesh->vertex_size);
			
			int influence_num;
			ifs.read(reinterpret_cast<char*>(&influence_num), sizeof(influence_num));
			int node;
			float weight;
			for (int k = 0; k < influence_num; ++k)
			{
				ifs.read(reinterpret_cast<char*>(&node), sizeof(node));
				ifs.read(reinterpret_cast<char*>(&weight), sizeof(weight));
				
				vertex->influence_nodes.push_back(node);
				vertex->influence_weights.push_back(weight);
			}
			
			mesh->vertices.push_back(vertex);
		}
		
		int idx_num;
		ifs.read(reinterpret_cast<char*>(&idx_num), sizeof(idx_num));
		int idx;
		for (int j = 0; j < idx_num; ++j)
		{
			ifs.read(reinterpret_cast<char*>(&idx), sizeof(idx));
			mesh->indices.push_back(idx);
		}
		
		ifs.read(reinterpret_cast<char*>(&mesh->vertex_format), sizeof(VertexFormat));
		
		skel->mesh_refs.push_back(mesh);
	}
	
	int anim_num;
	ifs.read(reinterpret_cast<char*>(&anim_num), sizeof(anim_num));
	for (int i = 0; i < anim_num; ++i)
	{
		AnimClip* anim = new AnimClip;
		
		int pose_num;
		ifs.read(reinterpret_cast<char*>(&pose_num), sizeof(pose_num));
		for (int j = 0; j < pose_num; ++j)
		{
			PoseSample* pose = new PoseSample;
			
			ifs.read(reinterpret_cast<char*>(&pose->skeleton_node_idx), sizeof(pose->skeleton_node_idx));
			
			int time_num;
			ifs.read(reinterpret_cast<char*>(&time_num), sizeof(time_num));
			float time_value;
			for (int k = 0; k < time_num; ++k)
			{
				ifs.read(reinterpret_cast<char*>(&time_value), sizeof(time_value));
				pose->times.push_back(time_value);
			}
			
			int transform_num;
			ifs.read(reinterpret_cast<char*>(&transform_num), sizeof(transform_num));
			Transform transform;
			for (int k = 0; k < transform_num; ++k)
			{
				ifs.read(reinterpret_cast<char*>(&transform), sizeof(Transform));
				pose->transforms.push_back(transform);
			}
			
			anim->pose_samples.push_back(pose);
		}
		
		skel->anim_refs.push_back(anim);
	}
	
	bool have_bounding;
	ifs.read(reinterpret_cast<char*>(&have_bounding), sizeof(have_bounding));
	if (have_bounding)
	{
		skel->bounding = new Sphere;
		
		ifs.read(reinterpret_cast<char*>(skel->bounding), sizeof(Sphere));
	}
	
	ifs.close();
	
	return skel;
}

}