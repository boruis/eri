/*
 *  skeleton_actor.cpp
 *  eri
 *
 *  Created by exe on 2010/11/3.
 *  Copyright 2010 cobbler. All rights reserved.
 *
 */

#include "skeleton_actor.h"

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
	
#pragma mark SkeletonNodeIns
	
	SkeletonNodeIns::SkeletonNodeIns() :
		current_start_key_(0),
		current_time_(0.0f)
	{
	}
	
	void SkeletonNodeIns::AddTime(float add_time, bool is_loop, bool is_inverse_)
	{
		if (!attached_sample)
			return;
		
		if (is_inverse_)
			current_time_ -= add_time;
		else
			current_time_ += add_time;

		UpdateKey(is_loop);
		UpdateLocalPose();
	}
	
	void SkeletonNodeIns::SetTime(float time)
	{
		if (!attached_sample)
			return;
		
		current_time_ = time;
		
		UpdateKey(false);
		UpdateLocalPose();
	}
	
	void SkeletonNodeIns::UpdateKey(bool is_loop)
	{
		ASSERT(attached_sample);
		
		int key_num = attached_sample->times.size();
		
		if (is_loop)
		{
			float duration = attached_sample->times[key_num - 1];
			while (current_time_ > duration) current_time_ -= duration;
			while (current_time_ < 0) current_time_ += duration;
		}

		int i = 0;
		for (; i < key_num; ++i)
		{
			if (current_time_ < attached_sample->times[i])
				break;
		}
		
		current_start_key_ = i - 1;
		next_start_key_ = i;
		
		if (current_start_key_ < 0) current_start_key_ = 0;
		if (next_start_key_ >= key_num) next_start_key_ = current_start_key_;
	}
	
	void SkeletonNodeIns::UpdateLocalPose()
	{
		ASSERT(attached_sample);
		
		if (current_start_key_ == next_start_key_)
		{
			local_pose = attached_sample->transforms[current_start_key_];
			return;
		}
		
		Transform& start = attached_sample->transforms[current_start_key_];
		Transform& end = attached_sample->transforms[next_start_key_];
		float start_time = attached_sample->times[current_start_key_];
		float end_time = attached_sample->times[next_start_key_];
		
		float blend_factor = (current_time_ - start_time) / (end_time - start_time);
		
		Quaternion::Slerp(local_pose.rotate, blend_factor, start.rotate, end.rotate);
		local_pose.scale = start.scale * (1.0f - blend_factor) + end.scale * blend_factor;
		local_pose.translate = start.translate * (1.0f - blend_factor) + end.translate * blend_factor;
	}
	
#pragma mark SkeletonIns

	SkeletonIns::SkeletonIns(SharedSkeleton* resource_ref) :
		resource_ref_(resource_ref),
		anim_speed_rate_(1.0f),
		is_inverse_(false),
		is_loop_(true)
	{
		ASSERT(resource_ref_);

		node_ins_array_.resize(resource_ref_->skeleton_ref->nodes.size());
	}

	SkeletonIns::~SkeletonIns()
	{
	}

	void SkeletonIns::SetAnim(int idx, float speed_rate, bool is_inverse, bool is_loop)
	{
		ASSERT(idx >= 0 && idx < resource_ref_->anim_refs.size());
		
		anim_idx_ = idx;
		anim_speed_rate_ = speed_rate;
		is_inverse_ = is_inverse;
		is_loop_ = is_loop;
		
		AttachSample();
	}

	void SkeletonIns::AddTime(float add_time)
	{
		float delta_time = add_time * anim_speed_rate_;
		
		int node_num = node_ins_array_.size();
		for (int i = 0; i < node_num; ++i)
		{
			node_ins_array_[i].AddTime(delta_time, is_loop_, is_inverse_);
		}
		
		UpdatePose();
	}
	
	void SkeletonIns::SetTimePercent(float time_percent)
	{
		float time = anim_duration_ * time_percent;
		
		int node_num = node_ins_array_.size();
		for (int i = 0; i < node_num; ++i)
		{
			node_ins_array_[i].SetTime(time);
		}
		
		UpdatePose();
	}
	
	int SkeletonIns::GetVertexBufferSize()
	{
		return (resource_ref_->mesh_refs[0]->vertex_size * resource_ref_->mesh_refs[0]->vertices.size());
	}
	
	int SkeletonIns::FillVertexBuffer(void* buffer)
	{
		unsigned char* buffer_data = static_cast<unsigned char*>(buffer);
		
		Mesh* mesh = resource_ref_->mesh_refs[0];
		Vertex* vertex;
		
		void* single_buffer = malloc(mesh->vertex_size);
		
		int vertex_num = mesh->vertices.size();
		for (int i = 0; i < vertex_num; ++i)
		{
			vertex = mesh->vertices[i];
			memcpy(single_buffer, vertex->data, mesh->vertex_size);
			
			float* float_value = static_cast<float*>(single_buffer);
			
			Vector3 pos(float_value[0], float_value[1], float_value[2]);
			Vector3 weight_pos, final_pos;
			
			// TODO: normal??
			//Vector3 normal(float_value[3], float_value[4], float_value[5])
			
			int influence_num = vertex->influence_nodes.size();
			for (int j = 0; j < influence_num; ++j)
			{
				weight_pos = node_ins_array_[vertex->influence_nodes[j]].matrix_palette * pos;
				weight_pos *= vertex->influence_weights[j];
				final_pos += weight_pos;
			}
			
			float_value[0] = final_pos.x;
			float_value[1] = final_pos.y;
			float_value[2] = final_pos.z;
			
			memcpy(&buffer_data[mesh->vertex_size * i], single_buffer, mesh->vertex_size);
		}
		
		free(single_buffer);
		
		return vertex_num;
	}
	
	void SkeletonIns::GetVertexInfo(GLenum& vertex_type, VertexFormat& vertex_format)
	{
		vertex_type = GL_TRIANGLES;
		vertex_format = resource_ref_->mesh_refs[0]->vertex_format;
	}

	void SkeletonIns::UpdatePose()
	{
		int node_num = node_ins_array_.size();
		SkeletonNode* node;

		Matrix4 temp_m;
		
		for (int i = 0; i < node_num; ++i)
		{
			SkeletonNodeIns& node_ins = node_ins_array_[i];
			node = resource_ref_->skeleton_ref->nodes[i];
			
			if (node_ins.attached_sample)
			{
				node_ins.global_matrix.MakeTransform(node_ins.local_pose.scale,
													 node_ins.local_pose.rotate,
													 node_ins.local_pose.translate);
			}
			else
			{
				node_ins.global_matrix = node->local_transform;
			}
			
			// retrive global transform from parent
			if (node->parent_idx != -1)
			{
				temp_m = node_ins_array_[node->parent_idx].global_matrix * node_ins.global_matrix;
				node_ins.global_matrix = temp_m;
			}
			
			if (node->is_joint)
				node_ins.matrix_palette = node_ins.global_matrix * node->inverse_bind_pose;
			else
				node_ins.matrix_palette = node_ins.global_matrix;
		}
	}
	
	void SkeletonIns::AttachSample()
	{
		int node_num = node_ins_array_.size();
		for (int i = 0; i < node_num; ++i)
		{
			node_ins_array_[i].attached_sample = NULL;
		}
		
		AnimClip* anim = resource_ref_->anim_refs[anim_idx_];
		PoseSample* pose_sample;
		
		anim_duration_ = 0.0f;
		float sample_duration;
		
		int sample_num = anim->pose_samples.size();
		for (int i = 0; i < sample_num; ++i)
		{
			pose_sample = anim->pose_samples[i];
			node_ins_array_[pose_sample->skeleton_node_idx].attached_sample = pose_sample;
			
			sample_duration = pose_sample->times[pose_sample->times.size() - 1];
			if (anim_duration_ < sample_duration)
				anim_duration_ = sample_duration;
		}
	}

#pragma mark SkeletonActor

	SkeletonActor::SkeletonActor(SharedSkeleton* resource_ref) :
		vertex_buffer_(NULL)
	{
		skeleton_ins_ = new SkeletonIns(resource_ref);
	}

	SkeletonActor::~SkeletonActor()
	{
		if (vertex_buffer_)
		{
			free(vertex_buffer_);
		}
		
		delete skeleton_ins_;
	}

	void SkeletonActor::Update(float delta_time)
	{
		skeleton_ins_->AddTime(delta_time);
		UpdateVertexBuffer();
	}
	
	void SkeletonActor::SetAnim(int idx, float speed_rate /*= 1.0f*/, bool is_inverse /*= false*/, bool is_loop /*= true*/)
	{
		skeleton_ins_->SetAnim(idx, speed_rate, is_inverse, is_loop);
	}
	
	void SkeletonActor::SetTimePercent(float time_percent)
	{
		skeleton_ins_->SetTimePercent(time_percent);
		UpdateVertexBuffer();
	}

	void SkeletonActor::UpdateVertexBuffer()
	{
		if (render_data_.vertex_buffer == 0)
		{
			glGenBuffers(1, &render_data_.vertex_buffer);
		}
		
		if (!vertex_buffer_)
		{
			vertex_buffer_size_ = skeleton_ins_->GetVertexBufferSize();
			
			ASSERT(vertex_buffer_size_ > 0);
			
			vertex_buffer_ = malloc(vertex_buffer_size_);
			
			ASSERT(vertex_buffer_);
		}
		
		render_data_.vertex_count = skeleton_ins_->FillVertexBuffer(vertex_buffer_);
		
		glBindBuffer(GL_ARRAY_BUFFER, render_data_.vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size_, vertex_buffer_, GL_DYNAMIC_DRAW);
	
		skeleton_ins_->GetVertexInfo(render_data_.vertex_type, render_data_.vertex_format);
	}

}
