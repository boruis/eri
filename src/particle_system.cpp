/*
 *  particle_system.cpp
 *  eri
 *
 *  Created by seedstudio03 on 2010/10/12.
 *  Copyright 2010 eri. All rights reserved.
 *
 */

#include "pch.h"

#include "particle_system.h"
#include "sys_helper.h"
#include "xml_helper.h"

using namespace rapidxml;

namespace ERI
{

	BaseEmitter::BaseEmitter(EmitterType type, float rate, float angle_min, float angle_max)
		: type_(type),
		angle_min_(angle_min),
		angle_max_(angle_max),
		align_angle_(false),
		emit_remain_time_(0.0f)
	{
		SetRate(rate);
	}

	BaseEmitter::~BaseEmitter()
	{
	}
	
	void BaseEmitter::SetRate(float rate)
	{
		ASSERT(rate > 0.0f);
		
		rate_ = rate;
		emit_interval_ = 1.0f / rate_;
	}

	void BaseEmitter::Restart()
	{
		emit_remain_time_ = 0.0f;
	}
	
	bool BaseEmitter::CheckIsTimeToEmit(float delta_time, int& out_emit_num)
	{
		emit_remain_time_ -= delta_time;
		if (emit_remain_time_ <= 0.0f)
		{
			out_emit_num = static_cast<int>(-emit_remain_time_ / emit_interval_) + 1;
			
			emit_remain_time_ = emit_interval_;
			return true;
		}
		
		return false;
	}

	float BaseEmitter::GetEmitAngle()
	{
		return RangeRandom(angle_min_, angle_max_);
	}

	BoxEmitter::BoxEmitter(const Vector2& half_size, float rate, float angle_min, float angle_max) :
		half_size_(half_size),
		BaseEmitter(EMITTER_BOX, rate, angle_min, angle_max)
	{
	}

	BoxEmitter::~BoxEmitter()
	{
	}
	
	BaseEmitter* BoxEmitter::Clone()
	{
		BaseEmitter* emitter = new BoxEmitter(half_size_, rate(), angle_min(), angle_max());
		emitter->set_align_angle(align_angle());
		return emitter;
	}

	void BoxEmitter::GetEmitPos(Vector2& pos) const
	{
		pos.x = RangeRandom(-half_size_.x, half_size_.x);
		pos.y = RangeRandom(-half_size_.y, half_size_.y);
	}
	
	CircleEmitter::CircleEmitter(float radius, float rate, float angle_min, float angle_max) :
		radius_(radius),
		BaseEmitter(EMITTER_CIRCLE, rate, angle_min, angle_max)
	{
	}
	
	CircleEmitter::~CircleEmitter()
	{
	}
	
	BaseEmitter* CircleEmitter::Clone()
	{
		BaseEmitter* emitter = new CircleEmitter(radius_, rate(), angle_min(), angle_max());
		emitter->set_align_angle(align_angle());
		return emitter;
	}
	
	void CircleEmitter::GetEmitPos(Vector2& pos) const
	{
		pos.x = RangeRandom(-radius_, radius_);
		pos.y = RangeRandom(-radius_, radius_);
		
		float radius_squared = radius_ * radius_;
		while (pos.LengthSquared() > radius_squared)
		{
			pos.x = RangeRandom(-radius_, radius_);
			pos.y = RangeRandom(-radius_, radius_);
		}
		
//		Vector2 r;
//		r.y = RangeRandom(0.0f, radius_);
//		r.Rotate(RangeRandom(0.0f, 360.0f));
//		pos.x = r.x;
//		pos.y = r.y;
	}
	
#pragma mark Affector
	
	RotateAffector::RotateAffector(float speed, float acceleration /*＝ 0.0f*/)
		: BaseAffector(AFFECTOR_ROTATE),
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
	
	BaseAffector* RotateAffector::Clone()
	{
		BaseAffector* affector = new RotateAffector(speed_, acceleration_);
		affector->set_period(period());
		return affector;
	}
	
	ForceAffector::ForceAffector(const Vector2& acceleration)
		: BaseAffector(AFFECTOR_FORCE),
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
	
	BaseAffector* ForceAffector::Clone()
	{
		BaseAffector* affector = new ForceAffector(acceleration_);
		affector->set_period(period());
		return affector;
	}
	
	AccelerationAffector::AccelerationAffector(float acceleration)
		: BaseAffector(AFFECTOR_ACCELERATION),
		acceleration_(acceleration)
	{
	}
	
	AccelerationAffector::~AccelerationAffector()
	{
	}
	
	void AccelerationAffector::Update(float delta_time, Particle* p)
	{
		Vector2 velocity_dir = p->velocity;
		
		float speed = velocity_dir.Normalize();
		float delta_speed = acceleration_ * delta_time;
		
		if ((speed + delta_speed) <= 0.f)
			p->velocity = Vector2::ZERO;
		else
			p->velocity += velocity_dir * (acceleration_ * delta_time);
	}
	
	BaseAffector* AccelerationAffector::Clone()
	{
		BaseAffector* affector = new AccelerationAffector(acceleration_);
		affector->set_period(period());
		return affector;
	}
	
	ScaleAffector::ScaleAffector(const Vector2& speed)
		: BaseAffector(AFFECTOR_SCALE),
		speed_(speed)
	{
	}
	
	ScaleAffector::~ScaleAffector()
	{
	}
	
	void ScaleAffector::Update(float delta_time, Particle* p)
	{
		p->scale += speed_ * delta_time;
		if (p->scale.x < 0.0f) p->scale.x = 0.0f;
		if (p->scale.y < 0.0f) p->scale.y = 0.0f;
	}
	
	BaseAffector* ScaleAffector::Clone()
	{
		BaseAffector* affector = new ScaleAffector(speed_);
		affector->set_period(period());
		return affector;
	}
	
	ColorAffector::ColorAffector(const Color& start, const Color& end)
		: BaseAffector(AFFECTOR_COLOR),
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
		: BaseAffector(AFFECTOR_COLOR_INTERVAL)
	{
	}
	
	ColorIntervalAffector::~ColorIntervalAffector()
	{
		for (int i = 0; i < intervals_.size(); ++i)
			delete intervals_[i];
	}
	
	void ColorIntervalAffector::InitSetup(Particle* p)
	{
		if (intervals_.empty())
			return;
		
		p->color = intervals_[0]->color;
		p->color_interval = 0;
	}
	
	void ColorIntervalAffector::Update(float delta_time, Particle* p)
	{
		if (p->color_interval >= (static_cast<int>(intervals_.size()) - 1))
			return;
		
		if (p->lived_percent <= intervals_[p->color_interval]->lived_percent)
			return;

		if (p->lived_percent >= intervals_[p->color_interval + 1]->lived_percent)
		{
			++p->color_interval;
		}
		
		if (p->color_interval >= intervals_.size() - 1)
		{
			p->color = intervals_[p->color_interval]->color;
		}
		else
		{
			float total = intervals_[p->color_interval + 1]->lived_percent - intervals_[p->color_interval]->lived_percent;
			float diff = p->lived_percent - intervals_[p->color_interval]->lived_percent;
			float diff_percent = diff / total;
			p->color = intervals_[p->color_interval]->color * (1.0f - diff_percent) + intervals_[p->color_interval + 1]->color * diff_percent;
		}
	}
	
	BaseAffector* ColorIntervalAffector::Clone()
	{
		ColorIntervalAffector* affector = new ColorIntervalAffector;
		
		for (int i = 0; i < intervals_.size(); ++i)
			affector->AddInterval(intervals_[i]->lived_percent, intervals_[i]->color);
		
		return affector;
	}
	
	void ColorIntervalAffector::AddInterval(float lived_percent, const Color& color)
	{
		ColorInterval* interval = new ColorInterval;
		interval->lived_percent = lived_percent;
		interval->color = color;
		intervals_.push_back(interval);
	}
	
	void ColorIntervalAffector::RemoveInterval(int idx)
	{
		ASSERT(idx >= 0 && idx < intervals_.size());
		
		delete intervals_[idx];
		intervals_.erase(intervals_.begin() + idx);
	}

#pragma mark ParticleSystem

	ParticleSystem::ParticleSystem(const ParticleSystemSetup* setup_ref) :
		setup_ref_(setup_ref),
		custom_life_(-1.0f),
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
		
		size_t num = particles_.size();
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
		
		custom_life_ = setup_ref_->life;

		render_data_.apply_identity_model_matrix = !setup_ref_->is_coord_relative;
	}

	void ParticleSystem::SetEmitter(BaseEmitter* emitter)
	{
		ASSERT(emitter);
		
		if (emitter_) delete emitter_;
		
		emitter_ = emitter;
		
		float life_max = Max(setup_ref_->particle_life_min, setup_ref_->particle_life_max);

//		int need_particle_num = Ceil(emitter_->rate() * life_max) + 1;
//		
//		if (setup_ref_->life > 0.f)
//			need_particle_num = Min(need_particle_num, Ceil(emitter_->rate() * setup_ref_->life) + 1);
		
		int need_particle_num = Max(static_cast<int>(emitter_->rate() * life_max * 1.25f), 1);
		int original_particle_num = static_cast<int>(particles_.size());
		
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
	
	void ParticleSystem::ClearAffectors()
	{
		for (int i = 0; i < affectors_.size(); ++i) delete affectors_[i];
		affectors_.clear();
	}
	
	void ParticleSystem::Play()
	{
		lived_time_ = 0.0f;
		emitter_->Restart();
	}
	
	bool ParticleSystem::IsPlaying()
	{
		return (custom_life_ < 0.0f || lived_time_ >= 0.0f);
	}

	void ParticleSystem::Update(float delta_time)
	{
		if (!IsPlaying())
			return;
		
		if (custom_life_ >= 0.0f)
		{
			lived_time_ += delta_time;
			if (lived_time_ > (custom_life_ + setup_ref_->particle_life_max))
				lived_time_ = -1.0f;
		}
		
		Particle* p;
		size_t num = particles_.size();
		size_t affector_num = affectors_.size();
		Vector2 delta_pos;
		
		system_scale_.x = 1.0f;
		system_scale_.y = 1.0f;
		if (!setup_ref_->is_coord_relative)
		{
			// TODO: 3D scale?
			system_scale_ = GetScale();
		}
		
		for (int i = 0; i < num; ++i)
		{
			p = particles_[i];
			if (p->in_use)
			{
				p->lived_time += delta_time;
				if (p->lived_time < p->life)
				{
					p->lived_percent = p->lived_time / p->life;
					
					delta_pos = p->velocity * system_scale_ * delta_time;
					p->pos.x += delta_pos.x;
					p->pos.y += delta_pos.y;
					
					for (int affector_idx = 0; affector_idx < affector_num; ++affector_idx)
					{
						float& affector_timer = p->affector_timers[affector_idx];
						
						if (affector_timer == -1.f || affector_timer > 0.f)
						{
							affectors_[affector_idx]->Update(delta_time, p);
							
							if (affector_timer > 0.f)
								affector_timer = Max(affector_timer - delta_time, 0.f);
						}
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
		
		int emit_num = 0;
		if ((custom_life_ < 0.0f || (lived_time_ > 0.0f && lived_time_ < custom_life_))
			&& emitter_->CheckIsTimeToEmit(delta_time, emit_num))
		{
			EmitParticle(emit_num);
		}
		
		UpdateBuffer();
	}
	
	void ParticleSystem::ResetParticles()
	{
		size_t num = particles_.size();
		for (int i = 0; i < num; ++i)
		{
			particles_[i]->Reset();
		}

		UpdateBuffer();
	}
		
	void ParticleSystem::EmitParticle(int num)
	{
		Particle* p;
		Vector2 pos;
		Vector2 v;
		float rotate;
		const SceneActor* inherit_actor;
		size_t affector_num = affectors_.size();

		for (int i = 0; i < num; ++i)
		{
			p = ObtainParticle();
			
			if (!p) return;
			
			emitter_->GetEmitPos(pos);
			rotate = emitter_->GetEmitAngle();
			
			if (!setup_ref_->is_coord_relative)
			{
				pos = Vector2(GetWorldTransform() * Vector3(pos));
				
				// TODO: 3D rotate?
				
				inherit_actor = this;
				while (inherit_actor)
				{
					rotate += inherit_actor->GetRotate();
					inherit_actor = inherit_actor->parent();
				}
			}
			
			p->pos = pos;
			
			p->size = setup_ref_->particle_size * RangeRandom(setup_ref_->particle_scale_min, setup_ref_->particle_scale_max);
			
			p->rotate_angle = RangeRandom(setup_ref_->particle_rotate_min, setup_ref_->particle_rotate_max);
			if (emitter_->align_angle())
				p->rotate_angle += rotate;
			
			p->life = RangeRandom(static_cast<float>(setup_ref_->particle_life_min),
                            static_cast<float>(setup_ref_->particle_life_max));

			v.x = 0.0f;
			v.y = 1.0f;
			v.Rotate(rotate);
			p->velocity = v * RangeRandom(setup_ref_->particle_speed_min, setup_ref_->particle_speed_max);
			
			for (int j = 0; j < affector_num; ++j)
			{
				affectors_[j]->InitSetup(p);
			}
			
			p->lived_time = 0.0f;
			p->lived_percent = 0.0f;
			p->in_use = true;
			
			p->affector_timers.resize(affectors_.size());
			for (int j = 0; j < affectors_.size(); ++j)
				p->affector_timers[j] = affectors_[j]->period();
		}
	}

	Particle* ParticleSystem::ObtainParticle()
	{
		size_t num = particles_.size();
		
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
		
		if (first_available_particle_idx_ < 0)
			return NULL;
		
		ASSERT(first_available_particle_idx_ < particles_.size()
			   && !particles_[first_available_particle_idx_]->in_use);
		
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
		int particle_num = static_cast<int>(particles_.size());
		
		if (render_data_.vertex_buffer == 0)
		{
			glGenBuffers(1, &render_data_.vertex_buffer);
		}
		
		int vertex_num = particle_num * 4;
		
		if (vertices_) delete [] vertices_;
		vertices_ = new vertex_2_pos_tex_color[vertex_num];
		memset(vertices_, 0, sizeof(vertex_2_pos_tex_color) * vertex_num);
		
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
		
		size_t num = particles_.size();
		int in_use_num = 0;
		
		Particle* p;
		
		Vector2 up, right;
		Color color;
		
		vertex_2_pos_tex_color* vertex = vertices_;
		
		for (int i = 0; i < num; ++i)
		{
			p = particles_[i];
			if (p->in_use)
			{
				color = p->color * render_data_.color;
				
				up.x = 0.0f;
				up.y = p->size.y * p->scale.y * 0.5f * system_scale_.y;
				up.Rotate(p->rotate_angle);
				
				right.x = p->size.x * p->scale.x * 0.5f * system_scale_.x;
				right.y = 0.0f;
				right.Rotate(p->rotate_angle);
				
				vertex->position[0] = p->pos.x + up.x - right.x;
				vertex->position[1] = p->pos.y + up.y - right.y;
				vertex->color[0] = static_cast<unsigned char>(color.r * 255.0f);
				vertex->color[1] = static_cast<unsigned char>(color.g * 255.0f);
				vertex->color[2] = static_cast<unsigned char>(color.b * 255.0f);
				vertex->color[3] = static_cast<unsigned char>(color.a * 255.0f);
				vertex->tex_coord[0] = uv_start_.x;
				vertex->tex_coord[1] = uv_start_.y;

				++vertex;
				
				vertex->position[0] = p->pos.x + up.x + right.x;
				vertex->position[1] = p->pos.y + up.y + right.y;
				vertex->color[0] = static_cast<unsigned char>(color.r * 255.0f);
				vertex->color[1] = static_cast<unsigned char>(color.g * 255.0f);
				vertex->color[2] = static_cast<unsigned char>(color.b * 255.0f);
				vertex->color[3] = static_cast<unsigned char>(color.a * 255.0f);
				vertex->tex_coord[0] = uv_start_.x + uv_size_.x;
				vertex->tex_coord[1] = uv_start_.y;

				++vertex;
				
				vertex->position[0] = p->pos.x - up.x - right.x;
				vertex->position[1] = p->pos.y - up.y - right.y;
				vertex->color[0] = static_cast<unsigned char>(color.r * 255.0f);
				vertex->color[1] = static_cast<unsigned char>(color.g * 255.0f);
				vertex->color[2] = static_cast<unsigned char>(color.b * 255.0f);
				vertex->color[3] = static_cast<unsigned char>(color.a * 255.0f);
				vertex->tex_coord[0] = uv_start_.x;
				vertex->tex_coord[1] = uv_start_.y + uv_size_.y;

				++vertex;
				
				vertex->position[0] = p->pos.x - up.x + right.x;
				vertex->position[1] = p->pos.y - up.y + right.y;
				vertex->color[0] = static_cast<unsigned char>(color.r * 255.0f);
				vertex->color[1] = static_cast<unsigned char>(color.g * 255.0f);
				vertex->color[2] = static_cast<unsigned char>(color.b * 255.0f);
				vertex->color[3] = static_cast<unsigned char>(color.a * 255.0f);
				vertex->tex_coord[0] = uv_start_.x + uv_size_.x;
				vertex->tex_coord[1] = uv_start_.y + uv_size_.y;
				
				++vertex;
				
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
	
#pragma mark ParticleSystemCreator
	
	ParticleSystemCreator::~ParticleSystemCreator()
	{
		if (setup) delete setup;
		if (emitter) delete emitter;
		for (int i = 0; i < affectors.size(); ++i)
		{
			delete affectors[i];
		}
	}
	
	ParticleSystem*	ParticleSystemCreator::Create()
	{
		ParticleSystem* ps = new ParticleSystem(setup);
		if (material_setup.tex_path.length() > 0)
		{
			ps->SetMaterial(material_setup.tex_path, material_setup.tex_filter, material_setup.tex_filter);
			ps->SetTexAreaUV(material_setup.u_start,
							 material_setup.v_start,
							 material_setup.u_width,
							 material_setup.v_height);
		}
		
		ps->SetDepthWrite(material_setup.depth_write);
		if (material_setup.blend_add)
			ps->BlendAdd();
		
		ASSERT(emitter);
		
		ps->SetEmitter(emitter->Clone());
		
		for (int i = 0; i < affectors.size(); ++i)
		{
			ps->AddAffector(affectors[i]->Clone());
		}

		ps->RefreshSetup();
		
		return ps;
	}
	
#pragma mark script loader function
	
	ParticleSystemCreator* LoadParticleSystemCreatorByScriptFile(const std::string& path)
	{
		XmlParseData parse_data;
		ParseFile(path, parse_data);
		
		ParticleSystemCreator* creator = new ParticleSystemCreator;
		creator->setup = new ParticleSystemSetup;
		
		xml_node<>* node;
		std::string s;
		
		node = parse_data.doc.first_node("particle_system");
		if (node)
		{
			GetAttrBool(node, "coord_related", creator->setup->is_coord_relative);
			GetAttrFloat(node, "life", creator->setup->life);
			
			node = node->first_node();
			while (node)
			{
				if (strcmp(node->name(), "particle") == 0)
				{
					xml_node<>* node2 = node->first_node();
					while (node2)
					{
						if (strcmp(node2->name(), "size") == 0)
						{
							GetAttrFloat(node2, "x", creator->setup->particle_size.x);
							GetAttrFloat(node2, "y", creator->setup->particle_size.y);
						}
						else if (strcmp(node2->name(), "life") == 0)
						{
							GetAttrFloat(node2, "min", creator->setup->particle_life_min);
							GetAttrFloat(node2, "max", creator->setup->particle_life_max);
						}
						else if (strcmp(node2->name(), "speed") == 0)
						{
							GetAttrFloat(node2, "min", creator->setup->particle_speed_min);
							GetAttrFloat(node2, "max", creator->setup->particle_speed_max);
						}
						else if (strcmp(node2->name(), "rotate") == 0)
						{
							GetAttrFloat(node2, "min", creator->setup->particle_rotate_min);
							GetAttrFloat(node2, "max", creator->setup->particle_rotate_max);
						}
						else if (strcmp(node2->name(), "scale") == 0)
						{
							GetAttrFloat(node2, "min", creator->setup->particle_scale_min);
							GetAttrFloat(node2, "max", creator->setup->particle_scale_max);
						}
						else if (strcmp(node2->name(), "rotate_affector") == 0)
						{
							float speed, acceleration;
							speed = acceleration = 0.0f;
							GetAttrFloat(node2, "speed", speed);
							GetAttrFloat(node2, "acceleration", acceleration);
							if (speed != 0.0f || acceleration != 0.0f)
							{
								RotateAffector* affector = new RotateAffector(speed, acceleration);
								float period;
								if (GetAttrFloat(node2, "period", period))
									affector->set_period(period);
								
								creator->affectors.push_back(affector);
							}
						}
						else if (strcmp(node2->name(), "force_affector") == 0)
						{
							Vector2 acceleration;
							GetAttrFloat(node2, "acceleration_x", acceleration.x);
							GetAttrFloat(node2, "acceleration_y", acceleration.y);
							if (acceleration.x != 0.0f || acceleration.y != 0.0f)
							{
								ForceAffector* affector = new ForceAffector(acceleration);
								float period;
								if (GetAttrFloat(node2, "period", period))
									affector->set_period(period);
								
								creator->affectors.push_back(affector);
							}					
						}
						else if (strcmp(node2->name(), "acceleration_affector") == 0)
						{
							float acceleration = 0.0f;
							GetAttrFloat(node2, "acceleration", acceleration);
							if (acceleration != 0.0f)
							{
								AccelerationAffector* affector = new AccelerationAffector(acceleration);
								float period;
								if (GetAttrFloat(node2, "period", period))
									affector->set_period(period);
								
								creator->affectors.push_back(affector);
							}
						}
						else if (strcmp(node2->name(), "scale_affector") == 0)
						{
							Vector2 speed;
							GetAttrFloat(node2, "speed_x", speed.x);
							GetAttrFloat(node2, "speed_y", speed.y);
							if (speed.x != 0.0f || speed.y != 0.0f)
							{
								ScaleAffector* affector = new ScaleAffector(speed);
								float period;
								if (GetAttrFloat(node2, "period", period))
									affector->set_period(period);
								
								creator->affectors.push_back(affector);
							}
						}
						else if (strcmp(node2->name(), "color_affector") == 0)
						{
							Color start, end;
							GetAttrColor(node2, "start", start);
							GetAttrColor(node2, "end", end);
							if (start != end)
							{
								creator->affectors.push_back(new ColorAffector(start, end));
							}
						}
						else if (strcmp(node2->name(), "color_interval_affector") == 0)
						{
							ColorIntervalAffector* affector = new ColorIntervalAffector;
							
							bool is_got_interval = false;
							
							xml_node<>* node3 = node2->first_node("interval");
							while (node3)
							{
								float lived_time = 0.0f;
								Color color;
								GetAttrFloat(node3, "lived_time", lived_time);
								GetAttrColor(node3, "color", color);
								affector->AddInterval(lived_time, color);
								
								is_got_interval = true;
								
								node3 = node3->next_sibling("interval");
							}
							
							if (is_got_interval)
								creator->affectors.push_back(affector);
							else
								delete affector;
						}
						
						node2 = node2->next_sibling();
					}
				}
				else if (strcmp(node->name(), "box_emitter") == 0)
				{
					Vector2 size;
					float rate = 1.0f;
					float angle_min = 0.0f;
					float angle_max = 0.0f;
					bool align_angle = false;
					GetAttrFloat(node, "size_x", size.x);
					GetAttrFloat(node, "size_y", size.y);
					GetAttrFloat(node, "rate", rate);
					GetAttrFloat(node, "angle_min", angle_min);
					GetAttrFloat(node, "angle_max", angle_max);
					GetAttrBool(node, "align_angle", align_angle);
					
					if (creator->emitter) delete creator->emitter;
					creator->emitter = new BoxEmitter(size * 0.5f, rate, angle_min, angle_max);
					creator->emitter->set_align_angle(align_angle);
				}
				else if (strcmp(node->name(), "circle_emitter") == 0)
				{
					float radius = 1.0f;
					float rate = 1.0f;
					float angle_min = 0.0f;
					float angle_max = 0.0f;
					bool align_angle = false;
					GetAttrFloat(node, "radius", radius);
					GetAttrFloat(node, "rate", rate);
					GetAttrFloat(node, "angle_min", angle_min);
					GetAttrFloat(node, "angle_max", angle_max);
					GetAttrBool(node, "align_angle", align_angle);
					
					if (creator->emitter) delete creator->emitter;
					creator->emitter = new CircleEmitter(radius, rate, angle_min, angle_max);
					creator->emitter->set_align_angle(align_angle);
				}
				else if (strcmp(node->name(), "material") == 0)
				{
					std::string s;
					xml_attribute<>* attr;
					
					attr = GetAttrStr(node, "tex", s);
					if (attr && s.length() > 0)
					{
						std::string absolute_path;
						
						if (s[0] == '/' || (s.length() >= 2 && s[1] == ':')) // already absolute path
						{
							absolute_path = s;
						}
						else
						{
							size_t pos = path.rfind('/');
							if (pos == std::string::npos)
								pos = path.rfind('\\');
							if (pos != std::string::npos)
								absolute_path = path.substr(0, pos);
							
							absolute_path += "/" + s;
						}

						creator->material_setup.tex_path = absolute_path;
						
						if (GetAttrStr(node, "tex_filter", s))
						{
							if (s.compare("nearest") == 0)
								creator->material_setup.tex_filter = FILTER_NEAREST;
						}
						
						GetAttrFloat(node, "tex_u", creator->material_setup.u_start);
						GetAttrFloat(node, "tex_v", creator->material_setup.v_start);
						GetAttrFloat(node, "tex_w", creator->material_setup.u_width);
						GetAttrFloat(node, "tex_h", creator->material_setup.v_height);
					}
					
					GetAttrBool(node, "depth_write", creator->material_setup.depth_write);
					
					if (GetAttrStr(node, "blend", s))
					{
						if (s.compare("add") == 0)
							creator->material_setup.blend_add = true;
					}
				}
				
				node = node->next_sibling();
			}
			
			if (!creator->emitter)
			{
				delete creator;
				creator = NULL;
			}
		}
		else
		{
			delete creator;
			creator = NULL;
		}

		return creator;
	}

	bool SaveParticleSystemToScriptByCreator(const ParticleSystemCreator* creator, const std::string& path)
	{
		ASSERT(creator);
		
		XmlCreateData data;
		
		// system
		
		ParticleSystemSetup default_setup;
		
		xml_node<>* node = CreateNode(data.doc, "particle_system");
		if (creator->setup->is_coord_relative != default_setup.is_coord_relative)
			PutAttrBool(data.doc, node, "coord_related", creator->setup->is_coord_relative);
		if (creator->setup->life != default_setup.life)
			PutAttrFloat(data.doc, node, "life", creator->setup->life);
		
		// material
		
		ParticleMaterialSetup default_material;
		
		bool modified = false;
		
		xml_node<>* material_node = CreateNode(data.doc, "material");
		
		if (!creator->material_setup.tex_path.empty())
		{
			std::string name = GetFileName(creator->material_setup.tex_path);
			PutAttrStr(data.doc, material_node, "tex", name);
			modified = true;
		}
		if (creator->material_setup.tex_filter == FILTER_NEAREST)
		{
			PutAttrStr(data.doc, material_node, "tex_filter", "nearest");
			modified = true;
		}
		if (creator->material_setup.u_start != default_material.u_start)
		{
			PutAttrFloat(data.doc, material_node, "tex_u", creator->material_setup.u_start);
			modified = true;
		}
		if (creator->material_setup.v_start != default_material.v_start)
		{
			PutAttrFloat(data.doc, material_node, "tex_v", creator->material_setup.v_start);
			modified = true;
		}
		if (creator->material_setup.u_width != default_material.u_width)
		{
			PutAttrFloat(data.doc, material_node, "tex_w", creator->material_setup.u_width);
			modified = true;
		}
		if (creator->material_setup.v_height != default_material.v_height)
		{
			PutAttrFloat(data.doc, material_node, "tex_h", creator->material_setup.v_height);
			modified = true;
		}
		if (creator->material_setup.depth_write != default_material.depth_write)
		{
			PutAttrBool(data.doc, material_node, "depth_write", creator->material_setup.depth_write);
			modified = true;
		}
		if (creator->material_setup.blend_add)
		{
			PutAttrStr(data.doc, material_node, "blend", "add");
			modified = true;
		}
		
		if (modified)
			node->append_node(material_node);
		
		// emitter
		
		ASSERT(creator->emitter);
		
		xml_node<>* emitter_node = NULL;
		
		if (creator->emitter->type() == EMITTER_BOX)
		{
			emitter_node = CreateNode(data.doc, "box_emitter");
			BoxEmitter* box_emitter = static_cast<BoxEmitter*>(creator->emitter);
			
			PutAttrFloat(data.doc, emitter_node, "size_x", box_emitter->half_size().x * 2);
			PutAttrFloat(data.doc, emitter_node, "size_y", box_emitter->half_size().y * 2);
		}
		else if (creator->emitter->type() == EMITTER_CIRCLE)
		{
			emitter_node = CreateNode(data.doc, "circle_emitter");
			CircleEmitter* circle_emitter = static_cast<CircleEmitter*>(creator->emitter);
			
			PutAttrFloat(data.doc, emitter_node, "radius", circle_emitter->radius());
		}
		
		ASSERT(emitter_node)
		
		PutAttrFloat(data.doc, emitter_node, "rate", creator->emitter->rate());
		PutAttrFloat(data.doc, emitter_node, "angle_min", creator->emitter->angle_min());
		PutAttrFloat(data.doc, emitter_node, "angle_max", creator->emitter->angle_max());
		PutAttrBool(data.doc, emitter_node, "align_angle", creator->emitter->align_angle());
		
		node->append_node(emitter_node);
		
		// particle
		
		xml_node<>* particle_node = CreateNode(data.doc, "particle");
		xml_node<>* node2;
		
		if (creator->setup->particle_size != default_setup.particle_size)
		{
			node2 = CreateNode(data.doc, "size");
			PutAttrFloat(data.doc, node2, "x", creator->setup->particle_size.x);
			PutAttrFloat(data.doc, node2, "y", creator->setup->particle_size.y);
			particle_node->append_node(node2);
		}
		
		if (creator->setup->particle_life_min != default_setup.particle_life_min ||
			creator->setup->particle_life_max != default_setup.particle_life_max)
		{
			node2 = CreateNode(data.doc, "life");
			PutAttrFloat(data.doc, node2, "min", creator->setup->particle_life_min);
			PutAttrFloat(data.doc, node2, "max", creator->setup->particle_life_max);
			particle_node->append_node(node2);
		}
		
		if (creator->setup->particle_speed_min != default_setup.particle_speed_min ||
			creator->setup->particle_speed_max != default_setup.particle_speed_max)
		{
			node2 = CreateNode(data.doc, "speed");
			PutAttrFloat(data.doc, node2, "min", creator->setup->particle_speed_min);
			PutAttrFloat(data.doc, node2, "max", creator->setup->particle_speed_max);
			particle_node->append_node(node2);
		}
		
		if (creator->setup->particle_rotate_min != default_setup.particle_rotate_min ||
			creator->setup->particle_rotate_max != default_setup.particle_rotate_max)
		{
			node2 = CreateNode(data.doc, "rotate");
			PutAttrFloat(data.doc, node2, "min", creator->setup->particle_rotate_min);
			PutAttrFloat(data.doc, node2, "max", creator->setup->particle_rotate_max);
			particle_node->append_node(node2);
		}
		
		if (creator->setup->particle_scale_min != default_setup.particle_scale_min ||
			creator->setup->particle_scale_max != default_setup.particle_scale_max)
		{
			node2 = CreateNode(data.doc, "scale");
			PutAttrFloat(data.doc, node2, "min", creator->setup->particle_scale_min);
			PutAttrFloat(data.doc, node2, "max", creator->setup->particle_scale_max);
			particle_node->append_node(node2);
		}
		
		node->append_node(particle_node);
		
		// affector
		
		for (int i = 0; i < creator->affectors.size(); ++i)
		{
			xml_node<>* affector_node = NULL;
			
			switch (creator->affectors[i]->type())
			{
				case AFFECTOR_ROTATE:
				{
					RotateAffector* rotate_affector = static_cast<RotateAffector*>(creator->affectors[i]);
					
					affector_node = CreateNode(data.doc, "rotate_affector");
					PutAttrFloat(data.doc, affector_node, "speed", rotate_affector->speed());
					PutAttrFloat(data.doc, affector_node, "acceleration", rotate_affector->acceleration());
					if (rotate_affector->period() > 0.f)
						PutAttrFloat(data.doc, affector_node, "period", rotate_affector->period());
				}
					break;
				case AFFECTOR_FORCE:
				{
					ForceAffector* force_affector = static_cast<ForceAffector*>(creator->affectors[i]);
					
					affector_node = CreateNode(data.doc, "force_affector");
					PutAttrFloat(data.doc, affector_node, "acceleration_x", force_affector->acceleration().x);
					PutAttrFloat(data.doc, affector_node, "acceleration_y", force_affector->acceleration().y);
					if (force_affector->period() > 0.f)
						PutAttrFloat(data.doc, affector_node, "period", force_affector->period());
				}
					break;
				case AFFECTOR_ACCELERATION:
				{
					AccelerationAffector* acceleration_affector = static_cast<AccelerationAffector*>(creator->affectors[i]);
					
					affector_node = CreateNode(data.doc, "acceleration_affector");
					PutAttrFloat(data.doc, affector_node, "acceleration", acceleration_affector->acceleration());
					if (acceleration_affector->period() > 0.f)
						PutAttrFloat(data.doc, affector_node, "period", acceleration_affector->period());
				}
					break;
				case AFFECTOR_SCALE:
				{
					ScaleAffector* scale_affector = static_cast<ScaleAffector*>(creator->affectors[i]);
					
					affector_node = CreateNode(data.doc, "scale_affector");
					PutAttrFloat(data.doc, affector_node, "speed_x", scale_affector->speed().x);
					PutAttrFloat(data.doc, affector_node, "speed_y", scale_affector->speed().y);
					if (scale_affector->period() > 0.f)
						PutAttrFloat(data.doc, affector_node, "period", scale_affector->period());
				}
					break;
				case AFFECTOR_COLOR:
				{
					ColorAffector* color_affector = static_cast<ColorAffector*>(creator->affectors[i]);
					
					affector_node = CreateNode(data.doc, "color_affector");
					PutAttrColor(data.doc, affector_node, "start", color_affector->start());
					PutAttrColor(data.doc, affector_node, "end", color_affector->end());
				}
					break;
				case AFFECTOR_COLOR_INTERVAL:
				{
					std::vector<ColorIntervalAffector::ColorInterval*>& intervals = static_cast<ColorIntervalAffector*>(creator->affectors[i])->intervals();
					
					if (!intervals.empty())
					{
						affector_node = CreateNode(data.doc, "color_interval_affector");
						
						for (int j = 0; j < intervals.size(); ++j)
						{
							node2 = CreateNode(data.doc, "interval");
							PutAttrFloat(data.doc, node2, "lived_time", intervals[j]->lived_percent);
							PutAttrColor(data.doc, node2, "color", intervals[j]->color);
							affector_node->append_node(node2);
						}
					}
				}
					break;
					
				default:
					ASSERT(0);
					break;
			}
			
			if (affector_node)
				particle_node->append_node(affector_node);
		}
		
		//
		
		data.doc.append_node(node);
		
		return SaveFile(path, data);
	}
	
}
