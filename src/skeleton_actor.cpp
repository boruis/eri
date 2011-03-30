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
		
		Vector3 min_pos, max_pos;
		Vector3 pos, weight_pos, final_pos;
		
		const Mesh* mesh = mesh_refs[0];
		Vertex* vertex;
		
		void* single_buffer = malloc(mesh->vertex_size);
		
		int vertex_num = mesh->vertices.size();
		for (int i = 0; i < vertex_num; ++i)
		{
			vertex = mesh->vertices[i];
			memcpy(single_buffer, vertex->data, mesh->vertex_size);
			
			float* float_value = static_cast<float*>(single_buffer);
			
			final_pos.x = float_value[0];
			final_pos.y = float_value[1];
			final_pos.z = float_value[2];
			
			if (i == 0)
			{
				min_pos = max_pos = final_pos;
			}
			else
			{
				if (final_pos.x < min_pos.x) min_pos.x = final_pos.x;
				if (final_pos.y < min_pos.y) min_pos.y = final_pos.y;
				if (final_pos.z < min_pos.z) min_pos.z = final_pos.z;
				if (final_pos.x > max_pos.x) max_pos.x = final_pos.x;
				if (final_pos.y > max_pos.y) max_pos.y = final_pos.y;
				if (final_pos.z > max_pos.z) max_pos.z = final_pos.z;
			}
		}
		
		free(single_buffer);
		
		bounding->center = (min_pos + max_pos) * 0.5f;
		pos = max_pos - min_pos;
		bounding->radius = Max(pos.x, pos.y);
		bounding->radius = Max(bounding->radius, pos.z);
		bounding->radius *= 0.5f;
		
		printf("bs center(%.2f, %.2f, %.2f) radius %.2f\n", bounding->center.x, bounding->center.y, bounding->center.z, bounding->radius);
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
			
			current_start_key_ = next_start_key_ = key_num - 1;
		}
		else if (next_start_key_ >= key_num)
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
		
		Quaternion::Slerp(local_pose.rotate, blend_factor, start.rotate, end.rotate, true);
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
		
		if (anim_current_time_ <= anim_duration_)
		{
			UpdatePose();
		}
	}
	
	float SkeletonIns::GetTime() const
	{
		return anim_current_time_;
	}
	
	bool SkeletonIns::IsAnimEnd() const
	{
		return (anim_current_time_ > anim_duration_);
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
	
	const Matrix4& SkeletonIns::GetNodeCurrentTransform(int idx) const
	{
		ASSERT(idx >= 0 && idx < node_ins_array_.size());
		
		return node_ins_array_[idx].global_matrix;
	}
	
	int SkeletonIns::GetAnimClipNum() const
	{
		return resource_ref_->anim_refs.size();
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
		int time_end_idx;
		
		int sample_num = anim->pose_samples.size();
		for (int i = 0; i < sample_num; ++i)
		{
			pose_sample = anim->pose_samples[i];
			node_ins_array_[pose_sample->skeleton_node_idx].attached_sample = pose_sample;
			node_ins_array_[pose_sample->skeleton_node_idx].SetTime(0.0f, anim_setting_);
			
			time_end_idx = pose_sample->times.size() - 1;
			if (!anim_setting_.is_blend_begin) time_end_idx -= 1;
			ASSERT(time_end_idx >= 0);
			
			sample_duration = pose_sample->times[time_end_idx];
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
		
		if (resource_ref->bounding)
		{
			bounding_sphere_ = new Sphere;
			bounding_sphere_world_ = new Sphere;
			*bounding_sphere_ = *resource_ref->bounding;
			*bounding_sphere_world_ = *resource_ref->bounding;
		}
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
		
		float time_percent = skeleton_ins_->GetTimePercent();
		
		delete skeleton_ins_;
		skeleton_ins_ = new SkeletonIns(resource_ref);
		
		skeleton_ins_->SetAnim(curr_anim_);
		skeleton_ins_->SetTimePercent(time_percent);
		
		UpdateVertexBuffer();
	}

	void SkeletonActor::Update(float delta_time)
	{
		skeleton_ins_->AddTime(delta_time);
		
		if (next_anim_.idx != -1 && skeleton_ins_->IsAnimEnd())
		{
			// TODO: use is_loop to recognize recover anim not a good idea ...
			
			if (!next_anim_.is_loop)
			{
				AnimSetting old_anim = curr_anim_;

				skeleton_ins_->SetAnim(next_anim_);
				curr_anim_ = next_anim_;
				next_anim_ = old_anim;
			}
			else
			{
				skeleton_ins_->SetAnim(next_anim_);
				curr_anim_ = next_anim_;
				next_anim_.idx = -1;
			}
		}
		
		if (!skeleton_ins_->IsAnimEnd())
		{
			UpdateVertexBuffer();
		}
	}
	
	void SkeletonActor::SetAnim(const AnimSetting& setting)
	{
		skeleton_ins_->SetAnim(setting);
		curr_anim_ = setting;
	}
	
	void SkeletonActor::SetTimePercent(float time_percent)
	{
		skeleton_ins_->SetTimePercent(time_percent);
		UpdateVertexBuffer();
	}
	
	void SkeletonActor::PlayAnim(const AnimSetting& setting, bool wait_current_finish /*= true*/)
	{
		if (setting == curr_anim_)
		{
			return;
		}
		
		if (wait_current_finish)
		{
			next_anim_ = setting;
			skeleton_ins_->CancelLoop();
		}
		else
		{
			if (curr_anim_.is_loop)
				next_anim_ = curr_anim_;
			
			SetAnim(setting);
		}
	}
	
	int	SkeletonActor::GetAnimIdx()
	{
		return curr_anim_.idx;
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
