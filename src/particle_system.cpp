/*
 *  particle_system.cpp
 *  eri
 *
 *  Created by seedstudio03 on 2010/10/12.
 *  Copyright 2010 eri. All rights reserved.
 *
 */

#include "particle_system.h"

namespace ERI
{

	BaseEmitter::BaseEmitter(float rate, float angle_min, float angle_max) :
		rate_(rate),
		angle_min_(angle_min),
		angle_max_(angle_max),
		emit_remain_time_(0.0f)
	{
		ASSERT(rate_ > 0);
		
		emit_interval_ = 1.0f / rate_;
	}

	BaseEmitter::~BaseEmitter()
	{
	}

	bool BaseEmitter::CheckIsTimeToEmit(float delta_time)
	{
		emit_remain_time_ -= delta_time;
		if (emit_remain_time_ <= 0.0f)
		{
			emit_remain_time_ = emit_interval_;
			return true;
		}
		
		return false;
	}

	float BaseEmitter::GetEmitAngle()
	{
		return RangeRandom(angle_min_, angle_max_);
	}

	BoxEmitter::BoxEmitter(Vector3 half_size, float rate, float angle_min, float angle_max) :
		half_size_(half_size),
		BaseEmitter(rate, angle_min, angle_max)
	{
	}

	BoxEmitter::~BoxEmitter()
	{
	}

	void BoxEmitter::GetEmitPos(Vector3& pos)
	{
		pos.x = RangeRandom(-half_size_.x, half_size_.x);
		pos.y = RangeRandom(-half_size_.y, half_size_.y);
		pos.z = RangeRandom(-half_size_.z, half_size_.z);
	}
	
#pragma mark Affector
	
	RotateAffector::RotateAffector(float speed, float acceleration /*＝ 0.0f*/) :
		speed_(speed),
		acceleration_(acceleration)
	{
	}
	
	RotateAffector::~RotateAffector()
	{
	}
	
	void RotateAffector::InitSetup(Particle* p)
	{
		p->rotate_speed = speed_;
	}
	
	void RotateAffector::Update(float delta_time, Particle* p)
	{
		p->rotate_speed += acceleration_ * delta_time;
		p->rotate_angle += p->rotate_speed * delta_time;
	}
	
	ForceAffector::ForceAffector(const Vector2& acceleration) :
		acceleration_(acceleration)
	{
	}
	
	ForceAffector::~ForceAffector()
	{
	}
	
	void ForceAffector::Update(float delta_time, Particle* p)
	{
		p->velocity += acceleration_ * delta_time;
	}
	
	AccelerationAffector::AccelerationAffector(float acceleration) :
		acceleration_(acceleration)
	{
	}
	
	AccelerationAffector::~AccelerationAffector()
	{
	}
	
	void AccelerationAffector::Update(float delta_time, Particle* p)
	{
		Vector2 velocity_dir = p->velocity;
		velocity_dir.Normalize();
		
		p->velocity += velocity_dir * (acceleration_ * delta_time);
	}
	
	ScaleAffector::ScaleAffector(const Vector2& speed) : speed_(speed)
	{
	}
	
	ScaleAffector::~ScaleAffector()
	{
	}
	
	void ScaleAffector::Update(float delta_time, Particle* p)
	{
		p->scale += speed_ * delta_time;
		if (p->scale.x < 0) p->scale.x = 0;
		if (p->scale.y < 0) p->scale.y = 0;
	}
	
	ColorAffector::ColorAffector(const Color& start, const Color& end) :
		start_(start),
		end_(end)
	{
	}
	
	ColorAffector::~ColorAffector()
	{
	}
	
	void ColorAffector::InitSetup(Particle* p)
	{
		p->color = start_;
	}
	
	void ColorAffector::Update(float delta_time, Particle* p)
	{
		p->color = start_ * (1.0f - p->lived_percent) + end_ * p->lived_percent;
	}
	
	ColorIntervalAffector::ColorIntervalAffector()
	{
	}
	
	ColorIntervalAffector::~ColorIntervalAffector()
	{
	}
	
	void ColorIntervalAffector::InitSetup(Particle* p)
	{
		ASSERT(intervals_.size() > 0);
		
		p->color = intervals_[0].color;
		p->color_interval = 0;
	}
	
	void ColorIntervalAffector::Update(float delta_time, Particle* p)
	{
		if (p->color_interval >= intervals_.size() - 1)
			return;
		
		if (p->lived_percent <= intervals_[p->color_interval].lived_percent)
			return;

		if (p->lived_percent >= intervals_[p->color_interval + 1].lived_percent)
		{
			++p->color_interval;
		}
		
		if (p->color_interval >= intervals_.size() - 1)
		{
			p->color = intervals_[p->color_interval].color;
		}
		else
		{
			float total = intervals_[p->color_interval + 1].lived_percent - intervals_[p->color_interval].lived_percent;
			float diff = p->lived_percent - intervals_[p->color_interval].lived_percent;
			float diff_percent = diff / total;
			p->color = intervals_[p->color_interval].color * (1.0f - diff_percent) + intervals_[p->color_interval + 1].color * diff_percent;
		}
	}
	
	void ColorIntervalAffector::AddInterval(float lived_percent, const Color& color)
	{
		ColorInterval interval;
		interval.lived_percent = lived_percent;
		interval.color = color;
		intervals_.push_back(interval);
	}
		
#pragma mark ParticleSystem

	ParticleSystem::ParticleSystem(const ParticleSystemSetup* setup_ref) :
		setup_ref_(setup_ref),
		emitter_(NULL),
		vertices_(NULL),
		indices_(NULL),
		uv_start_(Vector2(0.0f, 0.0f)),
		uv_size_(Vector2(1.0f, 1.0f)),
		lived_time_(-1.0f)
	{
		RefreshSetup();
	}

	ParticleSystem::~ParticleSystem()
	{
		if (indices_) delete [] indices_;
		if (vertices_) delete [] vertices_;
		
		int num = particles_.size();
		for (int i = 0; i < num; ++i)
		{
			delete particles_[i];
		}

		num = affectors_.size();
		for (int i = 0; i < num; ++i)
		{
			delete affectors_[i];
		}
		
		if (emitter_) delete emitter_;
	}
	
	void ParticleSystem::RefreshSetup()
	{
		ASSERT(setup_ref_);
		ASSERT(setup_ref_->particle_life_min <= setup_ref_->particle_life_max);

		render_data_.apply_identity_model_matrix = !setup_ref_->is_coord_relative;
	}

	void ParticleSystem::SetEmitter(BaseEmitter* emitter)
	{
		ASSERT(emitter);
		
		if (emitter_) delete emitter_;
		
		emitter_ = emitter;
		
		int need_particle_num = static_cast<int>(emitter_->rate() * setup_ref_->particle_life_max);
		int original_particle_num = particles_.size();
		
		for (int i = 0; i < need_particle_num; ++i)
		{
			if (i >= original_particle_num)
			{
				particles_.push_back(new Particle);
			}
			
			particles_[i]->Reset();
		}
		
		if (original_particle_num > need_particle_num)
		{
			for (int i = original_particle_num - 1; i >= need_particle_num; --i)
			{
				delete particles_[i];
				particles_.pop_back();
			}
		}
		
		first_available_particle_idx_ = 0;
		
		CreateBuffer();
	}

	void ParticleSystem::AddAffector(BaseAffector* affector)
	{
		ASSERT(affector);
		
		affectors_.push_back(affector);
	}
	
	void ParticleSystem::Play()
	{
		lived_time_ = 0.0f;
	}
	
	bool ParticleSystem::IsPlaying()
	{
		return (setup_ref_->life < 0.0f || lived_time_ >= 0.0f);
	}

	void ParticleSystem::Update(float delta_time)
	{
		if (!IsPlaying())
			return;
		
		if (setup_ref_->life >= 0.0f)
		{
			lived_time_ += delta_time;
			if (lived_time_ > (setup_ref_->life + setup_ref_->particle_life_max))
				lived_time_ = -1.0f;
		}
		
		Particle* p;
		int num = particles_.size();
		int affector_num = affectors_.size();
		
		for (int i = 0; i < num; ++i)
		{
			p = particles_[i];
			if (p->in_use)
			{
				p->lived_time += delta_time;
				if (p->lived_time < p->life)
				{
					p->lived_percent = p->lived_time / p->life;
					
					p->pos += p->velocity * delta_time;
					
					for (int affector_idx = 0; affector_idx < affector_num; ++affector_idx)
					{
						affectors_[affector_idx]->Update(delta_time, p);
					}
				}
				else
				{
					p->Reset();
					
					if (first_available_particle_idx_ > i || first_available_particle_idx_ < 0)
						first_available_particle_idx_ = i;
				}
			}
		}
		
		if ((setup_ref_->life < 0.0f || (lived_time_ > 0.0f && lived_time_ < setup_ref_->life))
			&& emitter_->CheckIsTimeToEmit(delta_time))
		{
			// TODO: delta_time need emit mutiple particle
			
			EmitParticle();
		}
		
		UpdateBuffer();
	}
		
	void ParticleSystem::EmitParticle()
	{
		Particle* p = ObtainParticle();

		Vector3 pos;
		Vector2 v(0, 1);
		emitter_->GetEmitPos(pos);
		float rotate = emitter_->GetEmitAngle();
		
		if (!setup_ref_->is_coord_relative)
		{
			pos = GetWorldTransform() * pos;

			// TODO: 3D rotate?

			const SceneActor* inherit_actor = this;
			while (inherit_actor)
			{
				rotate += inherit_actor->GetRotate();
				inherit_actor = inherit_actor->parent();
			}
		}
		
		p->pos = Vector2(pos.x, pos.y);
		
		p->size = setup_ref_->particle_size * RangeRandom(setup_ref_->particle_scale_min, setup_ref_->particle_scale_max);
		p->rotate_angle = RangeRandom(setup_ref_->particle_rotate_min, setup_ref_->particle_rotate_max);
		p->life = RangeRandom(setup_ref_->particle_life_min, setup_ref_->particle_life_max);
		
		v.Rotate(rotate);
		p->velocity = v * RangeRandom(setup_ref_->particle_speed_min, setup_ref_->particle_speed_max);
		
		int affector_num = affectors_.size();
		for (int i = 0; i < affector_num; ++i)
		{
			affectors_[i]->InitSetup(p);
		}
		
		p->lived_time = 0.0f;
		p->in_use = true;
	}

	Particle* ParticleSystem::ObtainParticle()
	{
		int num = particles_.size();
		
		if (first_available_particle_idx_ < 0)
		{
			for (int i = 0; i < num; ++i)
			{
				if (!particles_[i]->in_use)
				{
					first_available_particle_idx_ = i;
					break;
				}
			}
		}
		
		ASSERT(first_available_particle_idx_ >= 0 &&
			   first_available_particle_idx_ < particles_.size() &&
			   !particles_[first_available_particle_idx_]->in_use);
		
		Particle* p = particles_[first_available_particle_idx_];
		
		++first_available_particle_idx_;
		for (; first_available_particle_idx_ < num; ++first_available_particle_idx_)
		{
			if (!particles_[first_available_particle_idx_]->in_use)
			{
				break;
			}
		}
		
		if (first_available_particle_idx_ >= num)
			first_available_particle_idx_ = -1;
		
		return p;
	}

	void ParticleSystem::CreateBuffer()
	{
		int particle_num = particles_.size();
		
		if (render_data_.vertex_buffer == 0)
		{
			glGenBuffers(1, &render_data_.vertex_buffer);
		}
		
		int vertex_num = particle_num * 4;
		
		if (vertices_) delete [] vertices_;
		vertices_ = new vertex_2_pos_tex_color[vertex_num];
		
		glBindBuffer(GL_ARRAY_BUFFER, render_data_.vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_2_pos_tex_color) * vertex_num, vertices_, GL_DYNAMIC_DRAW);
		
		if (render_data_.index_buffer == 0)
		{
			glGenBuffers(1, &render_data_.index_buffer);
		}
		
		int index_num = particle_num * 6;
		
		if (indices_) delete [] indices_;
		indices_ = new unsigned short[index_num];
		
		//  0 -- 1
		//  |    |
		//  2 -- 3
		
		for (int i = 0; i < particle_num; ++i)
		{
			indices_[i * 6]		= i * 4;
			indices_[i * 6 + 1]	= i * 4 + 2;
			indices_[i * 6 + 2]	= i * 4 + 3;
			indices_[i * 6 + 3]	= i * 4 + 3;
			indices_[i * 6 + 4]	= i * 4 + 1;
			indices_[i * 6 + 5]	= i * 4;
		}
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_data_.index_buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * index_num, indices_, GL_STATIC_DRAW);
		
		render_data_.vertex_type = GL_TRIANGLES;
		render_data_.vertex_format = POS_TEX_COLOR_2;
		render_data_.vertex_count = 0;
		render_data_.index_count = 0;
	}


	void ParticleSystem::UpdateBuffer()
	{
		ASSERT(render_data_.vertex_buffer || render_data_.vertex_count == 0);
		ASSERT(render_data_.index_buffer || render_data_.index_count == 0);
		
		int num = particles_.size();
		int in_use_num = 0;
		int start_idx;
		
		Particle* p;
		
		Vector2 up, right;
		
		for (int i = 0; i < num; ++i)
		{
			p = particles_[i];
			if (p->in_use)
			{
				up.x = 0.0f;
				up.y = p->size.y * p->scale.y * 0.5f;
				up.Rotate(p->rotate_angle);
				
				right.x = p->size.x * p->scale.x * 0.5f;
				right.y = 0.0f;
				right.Rotate(p->rotate_angle);
				
				start_idx = in_use_num * 4;
				
				vertices_[start_idx].position[0] = p->pos.x + up.x - right.x;
				vertices_[start_idx].position[1] = p->pos.y + up.y - right.y;
				vertices_[start_idx].tex_coord[0] = uv_start_.x;
				vertices_[start_idx].tex_coord[1] = uv_start_.y;
				vertices_[start_idx].color[0] = p->color.r;
				vertices_[start_idx].color[1] = p->color.g;
				vertices_[start_idx].color[2] = p->color.b;
				vertices_[start_idx].color[3] = p->color.a;

				++start_idx;
				
				vertices_[start_idx].position[0] = p->pos.x + up.x + right.x;
				vertices_[start_idx].position[1] = p->pos.y + up.y + right.y;
				vertices_[start_idx].tex_coord[0] = uv_start_.x + uv_size_.x;
				vertices_[start_idx].tex_coord[1] = uv_start_.y;
				vertices_[start_idx].color[0] = p->color.r;
				vertices_[start_idx].color[1] = p->color.g;
				vertices_[start_idx].color[2] = p->color.b;
				vertices_[start_idx].color[3] = p->color.a;

				++start_idx;
				
				vertices_[start_idx].position[0] = p->pos.x - up.x - right.x;
				vertices_[start_idx].position[1] = p->pos.y - up.y - right.y;
				vertices_[start_idx].tex_coord[0] = uv_start_.x;
				vertices_[start_idx].tex_coord[1] = uv_start_.y + uv_size_.y;
				vertices_[start_idx].color[0] = p->color.r;
				vertices_[start_idx].color[1] = p->color.g;
				vertices_[start_idx].color[2] = p->color.b;
				vertices_[start_idx].color[3] = p->color.a;

				++start_idx;
				
				vertices_[start_idx].position[0] = p->pos.x - up.x + right.x;
				vertices_[start_idx].position[1] = p->pos.y - up.y + right.y;
				vertices_[start_idx].tex_coord[0] = uv_start_.x + uv_size_.x;
				vertices_[start_idx].tex_coord[1] = uv_start_.y + uv_size_.y;
				vertices_[start_idx].color[0] = p->color.r;
				vertices_[start_idx].color[1] = p->color.g;
				vertices_[start_idx].color[2] = p->color.b;
				vertices_[start_idx].color[3] = p->color.a;
				
				++in_use_num;
			}
		}
		
		glBindBuffer(GL_ARRAY_BUFFER, render_data_.vertex_buffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertex_2_pos_tex_color) * in_use_num * 4, vertices_);
		
		render_data_.vertex_count = in_use_num * 4;
		render_data_.index_count = in_use_num * 6;
	}

	void ParticleSystem::SetTexAreaUV(float start_u, float start_v, float width, float height)
	{
		uv_start_.x = start_u;
		uv_start_.y = start_v;
		uv_size_.x = width;
		uv_size_.y = height;
	}
	
}
