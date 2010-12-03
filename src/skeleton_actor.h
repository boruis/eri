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
	
	struct AnimSetting
	{
		AnimSetting(int _idx = -1,
					float _speed_rate = 1.0f,
					bool _is_loop = true,
					bool _is_blend_begin = true,
					bool _is_inverse = false)
		:
			idx(_idx),
			speed_rate(_speed_rate),
			is_loop(_is_loop),
			is_blend_begin(_is_blend_begin),
			is_inverse(_is_inverse)
		{
		}
		
		int		idx;
		float	speed_rate;
		bool	is_loop, is_blend_begin, is_inverse;
	};
	
	class SkeletonNodeIns
	{
	public:
		SkeletonNodeIns();
		
		// TODO: should put to another anim class interface ...
		
		void SetTime(float current_time, const AnimSetting& setting);
		
		//
		
		PoseSample*		attached_sample;
		Transform		local_pose;
		Matrix4			global_matrix;
		Matrix4			matrix_palette;
		
	private:
		
		// TODO: should put to another anim class interface ...
		
		void UpdateKey(float current_time, const AnimSetting& setting);
		void UpdateLocalPose(float current_time);

		int				current_start_key_, next_start_key_;
	};
	
	class SkeletonIns
	{
	public:
		SkeletonIns(const SharedSkeleton* resource_ref);
		~SkeletonIns();

		// TODO: should put to another anim class interface ...
		
		void SetAnim(const AnimSetting& setting);
		void GetAnim(AnimSetting& out_setting);
		void SetTimePercent(float time_percent);
		float GetTimePercent();
		void AddTime(float add_time);
		float GetTime();
		bool IsAnimEnd();
		void CancelLoop();
		
		//
		
		int GetVertexBufferSize();
		int FillVertexBuffer(void* buffer);
		void GetVertexInfo(GLenum& vertex_type, VertexFormat& vertex_format);

		void UpdatePose();

	private:
		void AttachSample();
		
		const SharedSkeleton*	resource_ref_;

		std::vector<SkeletonNodeIns>	node_ins_array_;
		
		// TODO: should put to another anim class interface ...
		
		AnimSetting		anim_setting_;
		float			anim_duration_;
		float			anim_current_time_;
	};
	
	class SkeletonActor : public SceneActor
	{
	public:
		SkeletonActor(const SharedSkeleton* resource_ref);
		virtual ~SkeletonActor();
		
		void ChangeResource(const SharedSkeleton* resource_ref);
		
		void Update(float delta_time);
		
		void SetAnim(const AnimSetting& setting);
		void SetTimePercent(float time_percent);
		
		void PlayAnimOnce(int idx, float speed_rate = 1.0f, bool is_inverse = false);
		
	private:
		void UpdateVertexBuffer();
		
		SkeletonIns*	skeleton_ins_;
		
		void*			vertex_buffer_;
		int				vertex_buffer_size_;

		AnimSetting		next_anim_;
	};
	
}

#endif // ERI_SKELETON_ACTOR
