//
//  shared_skeleton.h
//  dae2skelmesh
//
//  Created by chiayu on 2011/4/6.
//  Copyright 2011年 __MyCompanyName__. All rights reserved.
//

#ifndef ERI_SHARED_SKELETON_H
#define ERI_SHARED_SKELETON_H

#include <string>
#include <vector>

#include "render_data.h"
#include "math_helper.h"

namespace ERI
{

struct SkeletonNode
{
	SkeletonNode() : is_joint(false) {}
	
	std::string		name;
	int				parent_idx;
	Matrix4			local_transform;
	Matrix4			inverse_bind_pose;
	bool			is_joint;
};

struct Skeleton
{
	~Skeleton();
	
	std::vector<SkeletonNode*>	nodes;
};

struct Vertex
{
	Vertex() : data(NULL) {}
	~Vertex();
	
	void*				data;
	
	std::vector<int>	influence_nodes;
	std::vector<float>	influence_weights;
};

struct Mesh
{
	Mesh() : vertex_format(INVALID_VERTEX_FORMAT) {}
	~Mesh();
	
	std::vector<Vertex*>	vertices;
	std::vector<int>		indices;
	
	VertexFormat			vertex_format;
	int						vertex_size;
};

struct Transform
{
	Quaternion	rotate;
	Vector3		scale;
	Vector3		translate;
};

struct PoseSample
{
	PoseSample() : skeleton_node_idx(-1) {}
	
	int							skeleton_node_idx;
	std::vector<float>			times;
	std::vector<Transform>		transforms;
};

struct AnimClip
{
	~AnimClip();
	
	std::vector<PoseSample*>	pose_samples;
};

struct SharedSkeleton
{
	SharedSkeleton() : skeleton_ref(NULL), bounding(NULL) {}
	~SharedSkeleton();
	
	void CalculateBounding();
	
	Skeleton*					skeleton_ref;
	std::vector<Mesh*>			mesh_refs;
	std::vector<AnimClip*>		anim_refs;
	
	Sphere*						bounding;
};
	
#pragma mark binary SharedSkeleton save/load function
	
void SaveSharedSkeletonToBinaryFile(const SharedSkeleton* skel, const std::string& path);
SharedSkeleton* LoadSharedSkeletonFromBinaryFile(const std::string& path);

}

#endif // ERI_SHARED_SKELETON_H
