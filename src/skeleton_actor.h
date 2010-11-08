/*
 *  skeleton_actor.h
 *  eri
 *
 *  Created by exe on 2010/11/3.
 *  Copyright 2010 cobbler. All rights reserved.
 *
 */

#ifndef ERI_SKELETON_ACTOR
#define ERI_SKELETON_ACTOR

#include "scene_actor.h"

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
		~SharedSkeleton();
		
		Skeleton*					skeleton_ref;
		std::vector<Mesh*>			mesh_refs;
		std::vector<AnimClip*>		anim_refs;
	};
	
	class SkeletonNodeIns
	{
	public:
		SkeletonNodeIns();
		
		// TODO: should put to another anim class interface ...
		
		void AddTime(float add_time, bool is_loop, bool is_inverse);
		void SetTime(float time);
		
		//
		
		PoseSample*		attached_sample;
		Transform		local_pose;
		Matrix4			global_matrix;
		Matrix4			matrix_palette;
		
	private:
		
		// TODO: should put to another anim class interface ...
		
		void UpdateKey(bool is_loop);
		void UpdateLocalPose();

		int				current_start_key_, next_start_key_;
		float			current_time_;
	};
	
	class SkeletonIns
	{
	public:
		SkeletonIns(SharedSkeleton* resource_ref);
		~SkeletonIns();

		// TODO: should put to another anim class interface ...
		
		void SetAnim(int idx, float speed_rate, bool is_inverse, bool is_loop);
		void AddTime(float add_time);
		void SetTimePercent(float time_percent);
		
		//
		
		int GetVertexBufferSize();
		int FillVertexBuffer(void* buffer);
		void GetVertexInfo(GLenum& vertex_type, VertexFormat& vertex_format);

		void UpdatePose();

	private:
		void AttachSample();
		
		SharedSkeleton*		resource_ref_;

		std::vector<SkeletonNodeIns>	node_ins_array_;
		
		// TODO: should put to another anim class interface ...
		
		int		anim_idx_;
		float	anim_duration_;
		float	anim_speed_rate_;
		bool	is_inverse_, is_loop_;
	};
	
	class SkeletonActor : public SceneActor
	{
	public:
		SkeletonActor(SharedSkeleton* resource_ref);
		virtual ~SkeletonActor();
		
		void Update(float delta_time);
		
		void SetAnim(int idx, float speed_rate = 1.0f, bool is_inverse = false, bool is_loop = true);
		void SetTimePercent(float time_percent);
		
	private:
		void UpdateVertexBuffer();
		
		SkeletonIns*	skeleton_ins_;
		
		void*			vertex_buffer_;
		int				vertex_buffer_size_;
	};
	
}

#endif // ERI_SKELETON_ACTOR
