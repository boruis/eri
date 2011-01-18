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
		current_start_key_(0)
	{
	}
	
	void SkeletonNodeIns::SetTime(float current_time, const AnimSetting& setting)
	{
		if (!attached_sample)
			return;
		
		UpdateKey(current_time, setting);
		UpdateLocalPose(current_time);
	}
	
	void SkeletonNodeIns::UpdateKey(float current_time, const AnimSetting& setting)
	{
		ASSERT(attached_sample);
		
		int key_num = attached_sample->times.size();
		int i = 0;
		for (; i < key_num; ++i)
		{
			if (current_time < attached_sample->times[i])
				break;
		}
		
		current_start_key_ = i;
		next_start_key_ = i + 1;
		
		if (current_start_key_ >= key_num)
		{
			ASSERT(!setting.is_loop);
			
			current_start_key_ = key_num - 1;
			next_start_key_ = setting.is_blend_begin ? 0 : current_start_key_;
		}
		if (next_start_key_ >= key_num)
		{
			next_start_key_ = setting.is_blend_begin ? 0 : current_start_key_;
		}
	}
	
	void SkeletonNodeIns::UpdateLocalPose(float current_time)
	{
		ASSERT(attached_sample);
		
		if (current_start_key_ == next_start_key_)
		{
			local_pose = attached_sample->transforms[current_start_key_];
			return;
		}
		
		Transform& start = attached_sample->transforms[current_start_key_];
		Transform& end = attached_sample->transforms[next_start_key_];
		
		float start_time = 0;
		if (current_start_key_ > 0)
			start_time = attached_sample->times[current_start_key_ - 1];
		float end_time = attached_sample->times[current_start_key_];
		
		float blend_factor = (current_time - start_time) / (end_time - start_time);
		
		Quaternion::Slerp(local_pose.rotate, blend_factor, start.rotate, end.rotate);
		local_pose.scale = start.scale * (1.0f - blend_factor) + end.scale * blend_factor;
		local_pose.translate = start.translate * (1.0f - blend_factor) + end.translate * blend_factor;
	}
	
#pragma mark SkeletonIns

	SkeletonIns::SkeletonIns(const SharedSkeleton* resource_ref) :
		resource_ref_(resource_ref),
		anim_current_time_(0.0f)
	{
		ASSERT(resource_ref_);

		node_ins_array_.resize(resource_ref_->skeleton_ref->nodes.size());
	}

	SkeletonIns::~SkeletonIns()
	{
	}

	void SkeletonIns::SetAnim(const AnimSetting& setting)
	{
		ASSERT(setting.idx >= 0 && setting.idx < resource_ref_->anim_refs.size());
		
		anim_setting_ = setting;
		
		AttachSample();
	}
	
	void SkeletonIns::GetAnim(AnimSetting& out_setting)
	{
		out_setting = anim_setting_;
	}

	void SkeletonIns::SetTimePercent(float time_percent)
	{
		anim_current_time_ = anim_duration_ * time_percent;
		
		int node_num = node_ins_array_.size();
		for (int i = 0; i < node_num; ++i)
		{
			node_ins_array_[i].SetTime(anim_current_time_, anim_setting_);
		}
		
		UpdatePose();
	}
	
	float SkeletonIns::GetTimePercent() const
	{
		return anim_current_time_ / anim_duration_;
	}
	
	void SkeletonIns::AddTime(float add_time)
	{
		float delta_time = add_time * anim_setting_.speed_rate;
		
		if (anim_setting_.is_inverse)
			anim_current_time_ -= delta_time;
		else
			anim_current_time_ += delta_time;
		
		if (anim_setting_.is_loop)
		{
			while (anim_current_time_ >= anim_duration_) anim_current_time_ -= anim_duration_;
			while (anim_current_time_ < 0) anim_current_time_ += anim_duration_;
		}
		
		int node_num = node_ins_array_.size();
		for (int i = 0; i < node_num; ++i)
		{
			node_ins_array_[i].SetTime(anim_current_time_, anim_setting_);
		}
		
		UpdatePose();
	}
	
	float SkeletonIns::GetTime() const
	{
		return anim_current_time_;
	}
	
	bool SkeletonIns::IsAnimEnd() const
	{
		return (anim_current_time_ >= anim_duration_);
	}
	
	void SkeletonIns::CancelLoop()
	{
		anim_setting_.is_loop = false;
	}
	
	int SkeletonIns::FindNodeIdxByName(const std::string& name) const
	{
		const Skeleton* skeleton = resource_ref_->skeleton_ref;
		int num = skeleton->nodes.size();
		for (int i = 0; i < num; ++i)
		{
			if (skeleton->nodes[i]->name.compare(name) == 0)
				return i;
		}
		
		return -1;
	}
	
	Vector3 SkeletonIns::GetNodeCurrentWorldPos(int idx) const
	{
		ASSERT(idx >= 0 && idx < node_ins_array_.size());
		
		return node_ins_array_[idx].global_matrix.GetTranslate();
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
		
		AnimClip* anim = resource_ref_->anim_refs[anim_setting_.idx];
		PoseSample* pose_sample;
		
		anim_duration_ = 0.0f;
		float sample_duration;
		
		int sample_num = anim->pose_samples.size();
		for (int i = 0; i < sample_num; ++i)
		{
			pose_sample = anim->pose_samples[i];
			node_ins_array_[pose_sample->skeleton_node_idx].attached_sample = pose_sample;
			node_ins_array_[pose_sample->skeleton_node_idx].SetTime(0.0f, anim_setting_);
			
			sample_duration = pose_sample->times[pose_sample->times.size() - 1];
			if (anim_duration_ < sample_duration)
				anim_duration_ = sample_duration;
		}
		
		anim_current_time_ = 0.0f;
	}

#pragma mark SkeletonActor

	SkeletonActor::SkeletonActor(const SharedSkeleton* resource_ref) :
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
	
	void SkeletonActor::ChangeResource(const SharedSkeleton* resource_ref)
	{
		ASSERT(resource_ref);
		
		if (vertex_buffer_)
		{
			free(vertex_buffer_);
			vertex_buffer_ = NULL;
		}
		
		AnimSetting old_setting;
		skeleton_ins_->GetAnim(old_setting);
		float time_percent = skeleton_ins_->GetTimePercent();
		
		delete skeleton_ins_;
		skeleton_ins_ = new SkeletonIns(resource_ref);
		
		skeleton_ins_->SetAnim(old_setting);
		skeleton_ins_->SetTimePercent(time_percent);
		
		UpdateVertexBuffer();
	}

	void SkeletonActor::Update(float delta_time)
	{
		skeleton_ins_->AddTime(delta_time);
		
		if (next_anim_.idx != -1 && skeleton_ins_->IsAnimEnd())
		{
			AnimSetting old_setting;
			skeleton_ins_->GetAnim(old_setting);
			skeleton_ins_->SetAnim(next_anim_);
			
			// TODO: use is_loop to recognize recover anim not a good idea ...
			
			if (!next_anim_.is_loop)
			{
				next_anim_ = old_setting;
				next_anim_.is_loop = true;
			}
			else
			{
				next_anim_.idx = -1;
			}
		}
		
		UpdateVertexBuffer();
	}
	
	void SkeletonActor::SetAnim(const AnimSetting& setting)
	{
		skeleton_ins_->SetAnim(setting);
	}
	
	void SkeletonActor::SetTimePercent(float time_percent)
	{
		skeleton_ins_->SetTimePercent(time_percent);
		UpdateVertexBuffer();
	}
	
	void SkeletonActor::PlayAnim(int idx, float speed_rate /*= 1.0f*/, bool is_loop /*= false*/, bool is_inverse /*= false*/)
	{
		AnimSetting old_setting;
		skeleton_ins_->GetAnim(old_setting);
		
		if (idx == old_setting.idx
			&& speed_rate == old_setting.speed_rate
			&& is_inverse == old_setting.is_inverse)
		{
			return;
		}
		
		next_anim_.idx = idx;
		next_anim_.speed_rate = speed_rate;
		next_anim_.is_inverse = is_inverse;
		next_anim_.is_loop = is_loop;
		
		skeleton_ins_->CancelLoop();
	}
	
	int	SkeletonActor::GetAnimIdx()
	{
		AnimSetting setting;
		skeleton_ins_->GetAnim(setting);
		return setting.idx;
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
