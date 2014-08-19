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
#include "scene_mgr.h"
#include "sys_helper.h"
#include "xml_helper.h"

using namespace rapidxml;

namespace ERI
{

// -----------------------------------------------------------------------------
	
#pragma mark - Emitters

	BaseEmitter::BaseEmitter(EmitterType type, float rate, float angle_min, float angle_max)
		: type_(type),
		angle_min_(angle_min),
		angle_max_(angle_max),
		angle_base_from_center_(false),
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
	
// -----------------------------------------------------------------------------

	BoxEmitter::BoxEmitter(const Vector2& half_size, float rate, float angle_min, float angle_max) :
		half_size_(half_size),
		rotate_(0.f),
		BaseEmitter(EMITTER_BOX, rate, angle_min, angle_max)
	{
	}

	BoxEmitter::~BoxEmitter()
	{
	}
	
	BaseEmitter* BoxEmitter::Clone()
	{
		BoxEmitter* emitter = new BoxEmitter(half_size_, rate(), angle_min(), angle_max());
		emitter->set_rotate(rotate());
		emitter->set_offset(offset());
		emitter->set_angle_base_from_center(angle_base_from_center());
		emitter->set_align_angle(align_angle());
		return emitter;
	}

	void BoxEmitter::GetEmitPosAngle(ERI::Vector2 &out_pos, float &out_angle) const
	{
		out_pos.x = RangeRandom(-half_size_.x, half_size_.x);
		out_pos.y = RangeRandom(-half_size_.y, half_size_.y);
		
		if (rotate() != 0.f)
			out_pos.Rotate(rotate());
		
		out_angle = angle_base_from_center() ? Vector2::UNIT_Y.GetRotateToDegree(out_pos) : rotate();
		out_angle += RangeRandom(angle_min(), angle_max());
		
		out_pos += offset();
	}
	
// -----------------------------------------------------------------------------
	
	CircleEmitter::CircleEmitter(float radius, float rate, float angle_min, float angle_max) :
		radius_(radius),
		radius_min_(0.f),
		BaseEmitter(EMITTER_CIRCLE, rate, angle_min, angle_max)
	{
	}
	
	CircleEmitter::~CircleEmitter()
	{
	}
	
	BaseEmitter* CircleEmitter::Clone()
	{
		CircleEmitter* emitter = new CircleEmitter(radius_, rate(), angle_min(), angle_max());
		emitter->set_radius_min(radius_min());
		emitter->set_offset(offset());
		emitter->set_angle_base_from_center(angle_base_from_center());
		emitter->set_align_angle(align_angle());
		return emitter;
	}
	
	void CircleEmitter::GetEmitPosAngle(ERI::Vector2 &out_pos, float &out_angle) const
	{
		if (radius_min_ <= 0.f)
		{
			out_pos.x = RangeRandom(-radius_, radius_);
			out_pos.y = RangeRandom(-radius_, radius_);
			
			float radius_squared = radius_ * radius_;
			while (out_pos.LengthSquared() > radius_squared)
			{
				out_pos.x = RangeRandom(-radius_, radius_);
				out_pos.y = RangeRandom(-radius_, radius_);
			}
		}
		else
		{
			Vector2 r(0.f, RangeRandom(radius_min_, radius_));
			r.Rotate(RangeRandom(0.0f, 360.0f));
			out_pos.x = r.x;
			out_pos.y = r.y;
		}
		
		out_angle = angle_base_from_center() ? Vector2::UNIT_Y.GetRotateToDegree(out_pos) : 0.f;
		out_angle += RangeRandom(angle_min(), angle_max());
		
		out_pos += offset();
	}
	
// -----------------------------------------------------------------------------
	
#pragma mark - Affectors
	
	RotateAffector::RotateAffector(float speed, float acceleration /*＝ 0.0f*/)
		: BaseAffector(AFFECTOR_ROTATE),
		speed_(speed),
		acceleration_(acceleration)
	{
	}
	
	RotateAffector::~RotateAffector()
	{
	}
	
	void RotateAffector::InitSetup(ParticleSystem* owner, Particle* p)
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
		affector->set_delay(delay());
		affector->set_period(period());
		return affector;
	}
	
// -----------------------------------------------------------------------------
	
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
		affector->set_delay(delay());
		affector->set_period(period());
		return affector;
	}
	
// -----------------------------------------------------------------------------
	
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
		affector->set_delay(delay());
		affector->set_period(period());
		return affector;
	}
	
// -----------------------------------------------------------------------------
	
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
		affector->set_delay(delay());
		affector->set_period(period());
		return affector;
	}
	
// -----------------------------------------------------------------------------
	
	ColorAffector::ColorAffector(const Color& start, const Color& end)
		: BaseAffector(AFFECTOR_COLOR),
		start_(start),
		end_(end)
	{
	}
	
	ColorAffector::~ColorAffector()
	{
	}
	
	void ColorAffector::InitSetup(ParticleSystem* owner, Particle* p)
	{
		if (p->life > 0.f)
			p->color = start_;
	}
	
	void ColorAffector::Update(float delta_time, Particle* p)
	{
		if (p->life > 0.f)
			p->color = start_ * (1.0f - p->lived_percent) + end_ * p->lived_percent;
	}
	
// -----------------------------------------------------------------------------
	
	ColorIntervalAffector::ColorIntervalAffector()
		: BaseAffector(AFFECTOR_COLOR_INTERVAL)
	{
	}
	
	ColorIntervalAffector::~ColorIntervalAffector()
	{
		for (int i = 0; i < intervals_.size(); ++i)
			delete intervals_[i];
	}
	
	void ColorIntervalAffector::InitSetup(ParticleSystem* owner, Particle* p)
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
		
		float lived = p->life > 0.f ? p->lived_percent : p->lived_time;
		
		if (lived <= intervals_[p->color_interval]->lived)
			return;
		
		if (lived >= intervals_[p->color_interval + 1]->lived)
		{
			++p->color_interval;
		}
		
		if (p->color_interval >= intervals_.size() - 1)
		{
			p->color = intervals_[p->color_interval]->color;
		}
		else
		{
			float total = intervals_[p->color_interval + 1]->lived - intervals_[p->color_interval]->lived;
			float diff = lived - intervals_[p->color_interval]->lived;
			float diff_percent = diff / total;
			p->color = intervals_[p->color_interval]->color * (1.0f - diff_percent) + intervals_[p->color_interval + 1]->color * diff_percent;
		}
	}
	
	BaseAffector* ColorIntervalAffector::Clone()
	{
		ColorIntervalAffector* affector = new ColorIntervalAffector;
		
		for (int i = 0; i < intervals_.size(); ++i)
			affector->AddInterval(intervals_[i]->lived, intervals_[i]->color);
		
		return affector;
	}
	
	void ColorIntervalAffector::AddInterval(float lived, const Color& color)
	{
		ColorInterval* interval = new ColorInterval;
		interval->lived = lived;
		interval->color = color;
		intervals_.push_back(interval);
	}
	
	void ColorIntervalAffector::RemoveInterval(int idx)
	{
		ASSERT(idx >= 0 && idx < intervals_.size());
		
		delete intervals_[idx];
		intervals_.erase(intervals_.begin() + idx);
	}
	
// -----------------------------------------------------------------------------
	
	TextureUvAffector::TextureUvAffector(float u_speed, float v_speed, int coord_idx)
		: BaseAffector(AFFECTOR_TEXTURE_UV),
		u_speed_(u_speed),
		v_speed_(v_speed),
		coord_idx_(coord_idx)
	{
		ASSERT(coord_idx_ >= 0 && coord_idx_ < 2);
	}
	
    TextureUvAffector::~TextureUvAffector()
	{
	}
    
	void TextureUvAffector::Update(float delta_time, Particle* p)
	{
		p->uv_start[coord_idx_].x += u_speed_ * delta_time;
		p->uv_start[coord_idx_].y += v_speed_ * delta_time;
	}
	
	BaseAffector* TextureUvAffector::Clone()
	{
		BaseAffector* affector = new TextureUvAffector(u_speed_, v_speed_, coord_idx_);
		affector->set_delay(delay());
		affector->set_period(period());
		return affector;
	}
	
// -----------------------------------------------------------------------------
	
	AtlasAnimAffector::AtlasAnimAffector(float interval, bool loop, int coord_idx)
		: BaseAffector(AFFECTOR_ATLAS_ANIM),
		atlas_ref_(NULL),
		interval_(interval),
		loop_(loop),
		coord_idx_(coord_idx),
		tex_width_(0),
		tex_height_(0)
	{
	}
	
	AtlasAnimAffector::~AtlasAnimAffector()
	{
	}
	
	void AtlasAnimAffector::InitSetup(ParticleSystem* owner, Particle* p)
	{
		ASSERT(owner && p);
		
		const Texture* tex = owner->GetTexture(coord_idx_);
		if (tex)
		{
			tex_width_ = tex->width;
			tex_height_ = tex->height;
		}
		
		ApplyIdx(p, 0);
	}
    
    void AtlasAnimAffector::Update(float delta_time, Particle* p)
	{
		if (NULL == atlas_ref_ || interval_ <= 0.f)
			return;
		
		int idx = static_cast<int>((p->lived_time - delay()) / interval_);
		
		if (loop_)
			idx %= atlas_ref_->size();
		else
			idx = Min(idx, static_cast<int>(atlas_ref_->size()) - 1);
		
		if (idx != p->atlas_idx)
			ApplyIdx(p, idx);
	}
    
    BaseAffector* AtlasAnimAffector::Clone()
	{
		AtlasAnimAffector* affector = new AtlasAnimAffector(interval_, loop_, coord_idx_);
		affector->SetAtlas(atlas_res_, atlas_prefix_);
		affector->set_delay(delay());
		return affector;
	}
    
    void AtlasAnimAffector::SetAtlas(const std::string& res,
									 const std::string& prefix /*= ""*/)
	{
		atlas_res_ = res;
		atlas_prefix_ = prefix;
		atlas_ref_ = TextureAtlasMgr::Ins().GetArray(GetFileNameBase(res), prefix);
	}
	
	void AtlasAnimAffector::ApplyIdx(Particle* p, int idx)
	{
		p->atlas_idx = idx;

		if (NULL == atlas_ref_)
			return;
		
		ASSERT(idx >= 0 && idx < atlas_ref_->size());
		
		const TextureAtlasUnit& unit = (*atlas_ref_)[idx];
		
		if (tex_width_ > 0 && tex_height_ > 0)
		{
			p->uv_start[coord_idx_].x = static_cast<float>(unit.x) / tex_width_;
			p->uv_start[coord_idx_].y = static_cast<float>(unit.y) / tex_height_;
			p->uv_size[coord_idx_].x = static_cast<float>(unit.width) / tex_width_;
			p->uv_size[coord_idx_].y = static_cast<float>(unit.height) / tex_height_;
		}
	}
	
// -----------------------------------------------------------------------------

#pragma mark - ParticleSystem

	ParticleSystem::ParticleSystem(const ParticleSystemSetup* setup_ref) :
		setup_ref_(setup_ref),
		life_(-1.0f),
		emitter_(NULL),
		vertices_(NULL),
		indices_(NULL),
		lived_time_(-1.0f),
		delay_timer_(0.0f),
		emit_before_(false)
	{
		uv_size_[0] = uv_size_[1] = Vector2::UNIT;
		
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
	
	void ParticleSystem::AddToScene(int layer_id /*= 0*/)
	{
		SceneActor::AddToScene(layer_id);
		
		for (int i = 0; i < child_systems_.size(); ++i)
			child_systems_[i]->AddToScene(layer_id);
	}
	
	void ParticleSystem::RemoveFromScene()
	{
		SceneActor::RemoveFromScene();
		
		for (int i = 0; i < child_systems_.size(); ++i)
			child_systems_[i]->RemoveFromScene();
	}
	
	void ParticleSystem::RemoveChild(SceneActor* actor)
	{
		SceneActor::RemoveChild(actor);
		
		for (int i = static_cast<int>(child_systems_.size()) - 1; i >= 0; --i)
		{
			if (child_systems_[i] == actor)
				child_systems_.erase(child_systems_.begin() + i);
		}
	}
	
	void ParticleSystem::RemoveAllChilds()
	{
		SceneActor::RemoveAllChilds();
		
		child_systems_.clear();
	}
	
	void ParticleSystem::AddChildSystem(ParticleSystem* system)
	{
		ASSERT(system);
		
		AddChild(system);
		
		child_systems_.push_back(system);
		
		if (layer() != system->layer())
		{
			if (system->layer())
				system->RemoveFromScene();
			
			if (layer())
				system->AddToScene(layer()->id());
		}
	}
	
	void ParticleSystem::RefreshSetup()
	{
		ASSERT(setup_ref_);
		
		life_ = setup_ref_->life;
		
		particle_life_max_ = Max(setup_ref_->particle_life_min, setup_ref_->particle_life_max);
		if (setup_ref_->particle_life_min <= 0.f || setup_ref_->particle_life_max <= 0.f)
		{
			particle_life_max_ = -1.f;
			life_ = -1.f;
		}
		
		ASSERT(setup_ref_->life >= 0.f || particle_life_max_ > 0.f);

		render_data_.apply_identity_model_matrix = !setup_ref_->is_coord_relative;
		
		for (int i = 0; i < child_systems_.size(); ++i)
			child_systems_[i]->ResetParticles();
	}

	void ParticleSystem::SetEmitter(BaseEmitter* emitter)
	{
		ASSERT(emitter);
		
		if (emitter_) delete emitter_;
		
		emitter_ = emitter;
		
		int need_particle_num = 1;
		
		if (particle_life_max_ > 0.f)
			need_particle_num = Max(need_particle_num, Ceil(emitter_->rate() * particle_life_max_));
		
		if (setup_ref_->life > 0.f)
		{
			need_particle_num = need_particle_num > 1 ?
				Min(need_particle_num, Ceil(emitter_->rate() * setup_ref_->life)) :
				Max(need_particle_num, Ceil(emitter_->rate() * setup_ref_->life));
		}
		
//		float life_max = Max(setup_ref_->particle_life_min, setup_ref_->particle_life_max);
//		int need_particle_num = Max(static_cast<int>(emitter_->rate() * life_max * 1.25f), 1);
    
		int original_particle_num = static_cast<int>(particles_.size());
		
		for (int i = 0; i < need_particle_num; ++i)
		{
			if (i >= original_particle_num)
				particles_.push_back(new Particle);
			else
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
		delay_timer_ = setup_ref_->delay;
		emit_before_ = false;
		emitter_->Restart();
		
		for (int i = 0; i < child_systems_.size(); ++i)
			child_systems_[i]->Play();
	}
	
	bool ParticleSystem::IsPlaying()
	{
		if (life_ < 0.0f || lived_time_ >= 0.0f)
			return true;

		for (int i = 0; i < child_systems_.size(); ++i)
		{
			if (child_systems_[i]->IsPlaying())
				return true;
		}
		
		return false;
	}

	void ParticleSystem::Update(float delta_time)
	{
		if (!IsPlaying())
			return;
		
		for (int i = 0; i < child_systems_.size(); ++i)
			child_systems_[i]->Update(delta_time);
		
		if (delay_timer_ > 0.0f)
		{
			delay_timer_ -= delta_time;
			if (delay_timer_ > 0.0f)
				return;
		}
		
		if (life_ >= 0.0f)
		{
			lived_time_ += delta_time;
			if (lived_time_ > (life_ + particle_life_max_))
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
				if (p->lived_time < p->life || p->life <= 0.f)
				{
					p->lived_percent = p->life > 0.f ? p->lived_time / p->life : 0.f;
					
					delta_pos = p->velocity * system_scale_ * delta_time;
					p->pos.x += delta_pos.x;
					p->pos.y += delta_pos.y;
					
					for (int affector_idx = 0; affector_idx < affector_num; ++affector_idx)
					{
						Particle::AffectorVars& var = p->affector_vars[affector_idx];
						
						if (var.delay_timer > 0.f)
						{
							var.delay_timer -= delta_time;
						}
						else if (var.period_timer != 0.f)
						{
							affectors_[affector_idx]->Update(delta_time, p);
							
							if (var.period_timer > 0.f)
								var.period_timer = Max(var.period_timer - delta_time, 0.f);
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
		if (life_ < 0.0f || (lived_time_ > 0.0f && (!emit_before_ || lived_time_ < life_)))
		{
			float emit_delta_time = delta_time;
			
			if (life_ >= 0.0f && lived_time_ > life_)
				emit_delta_time = Max(emit_delta_time - (lived_time_ - life_), 0.0f);
			
			if (emitter_->CheckIsTimeToEmit(emit_delta_time, emit_num))
			{
				EmitParticle(emit_num);
				emit_before_ = true;
			}
		}
		
		UpdateBuffer();
	}
	
	void ParticleSystem::ResetParticles()
	{
		size_t num = particles_.size();
		for (int i = 0; i < num; ++i)
			particles_[i]->Reset();

		UpdateBuffer();
		
		for (int i = 0; i < child_systems_.size(); ++i)
			child_systems_[i]->ResetParticles();
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
			
			emitter_->GetEmitPosAngle(pos, rotate);
			
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
			
			p->uv_start[0] = uv_start_[0];
			p->uv_start[1] = uv_start_[1];
			p->uv_size[0] = uv_size_[0];
			p->uv_size[1] = uv_size_[1];
			
			p->lived_time = 0.0f;
			p->lived_percent = 0.0f;
			p->in_use = true;
			
			p->affector_vars.resize(affector_num);
			for (int j = 0; j < affector_num; ++j)
			{
				affectors_[j]->InitSetup(this, p);

				p->affector_vars[j].delay_timer = affectors_[j]->delay();
				p->affector_vars[j].period_timer = affectors_[j]->period();
			}
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
		vertices_ = new vertex_2_pos_tex2_color[vertex_num];
		memset(vertices_, 0, sizeof(vertex_2_pos_tex2_color) * vertex_num);
		
		glBindBuffer(GL_ARRAY_BUFFER, render_data_.vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_2_pos_tex2_color) * vertex_num, vertices_, GL_DYNAMIC_DRAW);
		
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
		render_data_.vertex_format = POS_TEX2_COLOR_2;
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
		
		// TODO: divide with / without uv2 version?
		
		vertex_2_pos_tex2_color* vertex = vertices_;
		
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
				vertex->tex_coord[0] = p->uv_start[0].x;
				vertex->tex_coord[1] = p->uv_start[0].y;
				vertex->tex_coord2[0] = p->uv_start[1].x;
				vertex->tex_coord2[1] = p->uv_start[1].y;

				++vertex;
				
				vertex->position[0] = p->pos.x + up.x + right.x;
				vertex->position[1] = p->pos.y + up.y + right.y;
				vertex->color[0] = static_cast<unsigned char>(color.r * 255.0f);
				vertex->color[1] = static_cast<unsigned char>(color.g * 255.0f);
				vertex->color[2] = static_cast<unsigned char>(color.b * 255.0f);
				vertex->color[3] = static_cast<unsigned char>(color.a * 255.0f);
				vertex->tex_coord[0] = p->uv_start[0].x + p->uv_size[0].x;
				vertex->tex_coord[1] = p->uv_start[0].y;
				vertex->tex_coord2[0] = p->uv_start[1].x + p->uv_size[1].x;
				vertex->tex_coord2[1] = p->uv_start[1].y;

				++vertex;
				
				vertex->position[0] = p->pos.x - up.x - right.x;
				vertex->position[1] = p->pos.y - up.y - right.y;
				vertex->color[0] = static_cast<unsigned char>(color.r * 255.0f);
				vertex->color[1] = static_cast<unsigned char>(color.g * 255.0f);
				vertex->color[2] = static_cast<unsigned char>(color.b * 255.0f);
				vertex->color[3] = static_cast<unsigned char>(color.a * 255.0f);
				vertex->tex_coord[0] = p->uv_start[0].x;
				vertex->tex_coord[1] = p->uv_start[0].y + p->uv_size[0].y;
				vertex->tex_coord2[0] = p->uv_start[1].x;
				vertex->tex_coord2[1] = p->uv_start[1].y + p->uv_size[1].y;

				++vertex;
				
				vertex->position[0] = p->pos.x - up.x + right.x;
				vertex->position[1] = p->pos.y - up.y + right.y;
				vertex->color[0] = static_cast<unsigned char>(color.r * 255.0f);
				vertex->color[1] = static_cast<unsigned char>(color.g * 255.0f);
				vertex->color[2] = static_cast<unsigned char>(color.b * 255.0f);
				vertex->color[3] = static_cast<unsigned char>(color.a * 255.0f);
				vertex->tex_coord[0] = p->uv_start[0].x + p->uv_size[0].x;
				vertex->tex_coord[1] = p->uv_start[0].y + p->uv_size[0].y;
				vertex->tex_coord2[0] = p->uv_start[1].x + p->uv_size[1].x;
				vertex->tex_coord2[1] = p->uv_start[1].y + p->uv_size[1].y;
				
				++vertex;
				
				++in_use_num;
			}
		}
		
		glBindBuffer(GL_ARRAY_BUFFER, render_data_.vertex_buffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertex_2_pos_tex2_color) * in_use_num * 4, vertices_);
		
		render_data_.vertex_count = in_use_num * 4;
		render_data_.index_count = in_use_num * 6;
	}

	void ParticleSystem::SetTexAreaUV(float start_u, float start_v, float width, float height, int coord_idx /*= 0*/)
	{
		ASSERT(coord_idx >= 0 && coord_idx < 2);
		
		uv_start_[coord_idx].x = start_u;
		uv_start_[coord_idx].y = start_v;
		uv_size_[coord_idx].x = width;
		uv_size_[coord_idx].y = height;
	}
	
// -----------------------------------------------------------------------------
	
#pragma mark - ParticleSystemCreator
	
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
		
		const Texture* tex;
		TextureEnvs envs;
		for (int i = 0; i < material_setup.units.size(); ++i)
		{
			ParticleMaterialUnit* unit = material_setup.units[i];
			tex = NULL;
			
			if (i == 0)
				tex = ps->SetMaterial(unit->path, unit->filter, unit->filter);
			else
				tex = ps->AddMaterial(unit->path, unit->filter, unit->filter);
			
			if (tex)
			{
				ps->SetTextureWrap(unit->wrap, unit->wrap, i);
				
				envs.mode = unit->env_mode;
				ps->SetTextureEnvs(envs, i);
				
				ps->SetTextureCoord(i, unit->coord_idx);
			}
		}
		
		for (int i = 0; i < 2; ++i)
		{
			ps->SetTexAreaUV(material_setup.uv_start[i].x,
							 material_setup.uv_start[i].y,
							 material_setup.uv_size[i].x,
							 material_setup.uv_size[i].y,
							 i);
		}
		
		ps->SetDepthWrite(material_setup.depth_write);
		ps->Blend(material_setup.blend_type);
		
		ASSERT(emitter);
		
		ps->SetEmitter(emitter->Clone());
		
		for (int i = 0; i < affectors.size(); ++i)
		{
			ps->AddAffector(affectors[i]->Clone());
		}

		ps->RefreshSetup();
		
		return ps;
	}
	
// -----------------------------------------------------------------------------
	
#pragma mark - script loader function
	
	ParticleSystemCreator* LoadParticleSystemCreator(xml_node<>* node, const std::string& absolute_dir)
	{
		ASSERT(node);
		
		ParticleSystemCreator* creator = new ParticleSystemCreator;
		creator->setup = new ParticleSystemSetup;
		
		GetAttrBool(node, "coord_related", creator->setup->is_coord_relative);
		GetAttrFloat(node, "life", creator->setup->life);
		GetAttrFloat(node, "delay", creator->setup->delay);
		
		std::string str;
		
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
							float delay;
							if (GetAttrFloat(node2, "delay", delay))
								affector->set_delay(delay);
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
							float delay;
							if (GetAttrFloat(node2, "delay", delay))
								affector->set_delay(delay);
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
							float delay;
							if (GetAttrFloat(node2, "delay", delay))
								affector->set_delay(delay);
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
							float delay;
							if (GetAttrFloat(node2, "delay", delay))
								affector->set_delay(delay);
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
					else if (strcmp(node2->name(), "texture_uv_affector") == 0)
					{
						Vector2 speed;
						GetAttrFloat(node2, "speed_u", speed.x);
						GetAttrFloat(node2, "speed_v", speed.y);
						int coord_idx = 0;
						GetAttrInt(node2, "coord_idx", coord_idx);
						if (speed.x != 0.0f || speed.y != 0.0f)
						{
							TextureUvAffector* affector = new TextureUvAffector(speed.x, speed.y, coord_idx);
							
							float delay;
							if (GetAttrFloat(node2, "delay", delay))
								affector->set_delay(delay);
							float period;
							if (GetAttrFloat(node2, "period", period))
								affector->set_period(period);
							
							creator->affectors.push_back(affector);
						}
					}
					else if (strcmp(node2->name(), "atlas_anim_affector") == 0)
					{
						std::string res, prefix;
						GetAttrStr(node2, "atlas", res);
						GetAttrStr(node2, "prefix", prefix);
						int coord_idx = 0;
						GetAttrInt(node2, "coord_idx", coord_idx);
						float interval = 0.1f;
						GetAttrFloat(node2, "interval", interval);
						bool loop = false;
						GetAttrBool(node2, "loop", loop);
						if (!res.empty())
						{
							AtlasAnimAffector* affector = new AtlasAnimAffector(interval, loop, coord_idx);
							
							std::string absolute_path;
							if (res[0] == '/' || (res.length() >= 2 && res[1] == ':')) // already absolute path
								absolute_path = res;
							else
								absolute_path = absolute_dir + res;
							
							affector->SetAtlas(absolute_path, prefix);
							
							float delay;
							if (GetAttrFloat(node2, "delay", delay))
								affector->set_delay(delay);

							creator->affectors.push_back(affector);
						}
					}
					
					node2 = node2->next_sibling();
				}
			}
			else if (strcmp(node->name(), "box_emitter") == 0)
			{
				Vector2 size;
				Vector2 offset;
				float rotate = 0.f;
				float rate = 1.0f;
				float angle_min = 0.0f;
				float angle_max = 0.0f;
				bool align_angle = false;
				GetAttrFloat(node, "size_x", size.x);
				GetAttrFloat(node, "size_y", size.y);
				GetAttrFloat(node, "rotate", rotate);
				GetAttrVector2(node, "offset", offset);
				GetAttrFloat(node, "rate", rate);
				GetAttrFloat(node, "angle_min", angle_min);
				GetAttrFloat(node, "angle_max", angle_max);
				GetAttrBool(node, "align_angle", align_angle);
				
				if (creator->emitter) delete creator->emitter;
				
				BoxEmitter* box_emitter = new BoxEmitter(size * 0.5f, rate, angle_min, angle_max);
				box_emitter->set_rotate(rotate);
				
				creator->emitter = box_emitter;
				creator->emitter->set_offset(offset);
				creator->emitter->set_align_angle(align_angle);
			}
			else if (strcmp(node->name(), "circle_emitter") == 0)
			{
				float radius = 1.0f;
				Vector2 offset;
				float rate = 1.0f;
				float angle_min = 0.0f;
				float angle_max = 0.0f;
				bool align_angle = false;
				GetAttrFloat(node, "radius", radius);
				GetAttrVector2(node, "offset", offset);
				GetAttrFloat(node, "rate", rate);
				GetAttrFloat(node, "angle_min", angle_min);
				GetAttrFloat(node, "angle_max", angle_max);
				GetAttrBool(node, "align_angle", align_angle);
				
				if (creator->emitter) delete creator->emitter;
				
				creator->emitter = new CircleEmitter(radius, rate, angle_min, angle_max);
				creator->emitter->set_offset(offset);
				creator->emitter->set_align_angle(align_angle);
			}
			else if (strcmp(node->name(), "material") == 0 &&
					 creator->material_setup.units.size() < MAX_TEXTURE_UNIT)
			{
				xml_attribute<>* attr;
				
				ParticleMaterialSetup& material = creator->material_setup;
				material.units.push_back(new ParticleMaterialUnit);
				
				ParticleMaterialUnit* unit = material.units.back();

				attr = GetAttrStr(node, "tex", str);
				if (attr && str.length() > 0)
				{
					std::string absolute_path;
					
					if (str[0] == '/' || (str.length() >= 2 && str[1] == ':')) // already absolute path
						absolute_path = str;
					else
						absolute_path = absolute_dir + str;
					
					unit->path = absolute_path;
					
					if (GetAttrStr(node, "tex_filter", str))
					{
						if (str.compare("nearest") == 0)
							unit->filter = FILTER_NEAREST;
					}
					
					if (GetAttrStr(node, "tex_wrap", str))
					{
						if (str.compare("clamp") == 0)
							unit->wrap = WRAP_CLAMP_TO_EDGE;
					}
					
					if (GetAttrStr(node, "tex_env_mode", str))
					{
						if (str.compare("replace") == 0)
							unit->env_mode = MODE_REPLACE;
						else if (str.compare("decal") == 0)
							unit->env_mode = MODE_DECAL;
						else if (str.compare("blend") == 0)
							unit->env_mode = MODE_BLEND;
						else if (str.compare("add") == 0)
							unit->env_mode = MODE_ADD;
					}
					
					GetAttrInt(node, "coord_idx", unit->coord_idx);
				}
				
				ASSERT(unit->coord_idx < 2);
				
				GetAttrFloat(node, "tex_u", material.uv_start[unit->coord_idx].x);
				GetAttrFloat(node, "tex_v", material.uv_start[unit->coord_idx].y);
				GetAttrFloat(node, "tex_w", material.uv_size[unit->coord_idx].x);
				GetAttrFloat(node, "tex_h", material.uv_size[unit->coord_idx].y);

				if (creator->material_setup.units.size() == 1) // first material
				{
					GetAttrBool(node, "depth_write", material.depth_write);
					
					if (GetAttrStr(node, "blend", str))
					{
						if (str.compare("add") == 0)
							material.blend_type = BLEND_ADD;
						else if (str.compare("multiply") == 0)
							material.blend_type = BLEND_MULTIPLY;
						else if (str.compare("multiply2x") == 0)
							material.blend_type = BLEND_MULTIPLY2X;
						else if (str.compare("replace") == 0)
							material.blend_type = BLEND_REPLACE;
					}
				}
			}
			
			node = node->next_sibling();
		}
		
		if (!creator->emitter)
		{
			delete creator;
			creator = NULL;
		}

		return creator;
	}
	
	void SaveParticleSystem(const ParticleSystemCreator* creator, XmlCreateData& data)
	{
		ASSERT(creator);
    
		// system
		
		ParticleSystemSetup default_setup;
		
		xml_node<>* node = CreateNode(data.doc, "particle_system");
		
		if (creator->setup->is_coord_relative != default_setup.is_coord_relative)
			PutAttrBool(data.doc, node, "coord_related", creator->setup->is_coord_relative);
		if (creator->setup->life != default_setup.life)
			PutAttrFloat(data.doc, node, "life", creator->setup->life);
		if (creator->setup->delay != default_setup.delay)
			PutAttrFloat(data.doc, node, "delay", creator->setup->delay);
		
		// material
		
		ParticleMaterialSetup default_material;
		ParticleMaterialUnit default_material_unit;
		
		const ParticleMaterialSetup& material = creator->material_setup;
		for (int i = 0; i < material.units.size(); ++i)
		{
			const ParticleMaterialUnit* unit = material.units[i];
			
			xml_node<>* material_node = CreateNode(data.doc, "material");
			
			if (!unit->path.empty())
			{
				std::string name = GetFileName(unit->path);
				PutAttrStr(data.doc, material_node, "tex", name);
			}
			
			if (unit->filter == FILTER_NEAREST)
				PutAttrStr(data.doc, material_node, "tex_filter", "nearest");
			
			if (unit->wrap == WRAP_CLAMP_TO_EDGE)
				PutAttrStr(data.doc, material_node, "tex_wrap", "clamp");
			
			if (unit->env_mode == MODE_REPLACE)
				PutAttrStr(data.doc, material_node, "tex_env_mode", "replace");
			else if (unit->env_mode == MODE_DECAL)
				PutAttrStr(data.doc, material_node, "tex_env_mode", "decal");
			else if (unit->env_mode == MODE_BLEND)
				PutAttrStr(data.doc, material_node, "tex_env_mode", "blend");
			else if (unit->env_mode == MODE_ADD)
				PutAttrStr(data.doc, material_node, "tex_env_mode", "add");
			
			Vector2 uv_start = material.uv_start[unit->coord_idx];
			Vector2 uv_size = material.uv_size[unit->coord_idx];
			
			if (uv_start.x != default_material.uv_start[unit->coord_idx].x)
				PutAttrFloat(data.doc, material_node, "tex_u", uv_start.x);
			
			if (uv_start.y != default_material.uv_start[unit->coord_idx].y)
				PutAttrFloat(data.doc, material_node, "tex_v", uv_start.y);
			
			if (uv_size.x != default_material.uv_size[unit->coord_idx].x)
				PutAttrFloat(data.doc, material_node, "tex_w", uv_size.x);
			
			if (uv_size.y != default_material.uv_size[unit->coord_idx].y)
				PutAttrFloat(data.doc, material_node, "tex_h", uv_size.y);
			
			if (unit->coord_idx != default_material_unit.coord_idx)
				PutAttrInt(data.doc, material_node, "coord_idx", unit->coord_idx);
			
			if (i == 0)
			{
				if (material.depth_write != default_material.depth_write)
					PutAttrBool(data.doc, material_node, "depth_write", material.depth_write);
				
				if (material.blend_type != default_material.blend_type)
				{
					switch (material.blend_type)
					{
						case BLEND_ADD: PutAttrStr(data.doc, material_node, "blend", "add"); break;
						case BLEND_MULTIPLY: PutAttrStr(data.doc, material_node, "blend", "multiply"); break;
						case BLEND_MULTIPLY2X: PutAttrStr(data.doc, material_node, "blend", "multiply2x"); break;
						case BLEND_REPLACE: PutAttrStr(data.doc, material_node, "blend", "replace"); break;
						default: ASSERT(0); break;
					}
				}
			}

			node->append_node(material_node);
		}
		
		// emitter
		
		ASSERT(creator->emitter);
		
		xml_node<>* emitter_node = NULL;
		
		if (creator->emitter->type() == EMITTER_BOX)
		{
			emitter_node = CreateNode(data.doc, "box_emitter");
			BoxEmitter* box_emitter = static_cast<BoxEmitter*>(creator->emitter);
			
			PutAttrFloat(data.doc, emitter_node, "size_x", box_emitter->half_size().x * 2);
			PutAttrFloat(data.doc, emitter_node, "size_y", box_emitter->half_size().y * 2);
			
			if (box_emitter->rotate() != 0.f)
				PutAttrFloat(data.doc, emitter_node, "rotate", box_emitter->rotate());
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
		
		if (creator->emitter->offset().LengthSquared() > 0.f)
			PutAttrVector2(data.doc, emitter_node, "offset", creator->emitter->offset());
		
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
						if (rotate_affector->delay() > 0.f)
							PutAttrFloat(data.doc, affector_node, "delay", rotate_affector->delay());
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
						if (force_affector->delay() > 0.f)
							PutAttrFloat(data.doc, affector_node, "delay", force_affector->delay());
						if (force_affector->period() > 0.f)
							PutAttrFloat(data.doc, affector_node, "period", force_affector->period());
					}
					break;
					
				case AFFECTOR_ACCELERATION:
					{
						AccelerationAffector* acceleration_affector = static_cast<AccelerationAffector*>(creator->affectors[i]);
						
						affector_node = CreateNode(data.doc, "acceleration_affector");
						PutAttrFloat(data.doc, affector_node, "acceleration", acceleration_affector->acceleration());
						if (acceleration_affector->delay() > 0.f)
							PutAttrFloat(data.doc, affector_node, "delay", acceleration_affector->delay());
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
						if (scale_affector->delay() > 0.f)
							PutAttrFloat(data.doc, affector_node, "delay", scale_affector->delay());
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
								PutAttrFloat(data.doc, node2, "lived_time", intervals[j]->lived);
								PutAttrColor(data.doc, node2, "color", intervals[j]->color);
								affector_node->append_node(node2);
							}
						}
					}
					break;
					
				case AFFECTOR_TEXTURE_UV:
					{
						TextureUvAffector* texture_uv_affector = static_cast<TextureUvAffector*>(creator->affectors[i]);
						
						affector_node = CreateNode(data.doc, "texture_uv_affector");
						PutAttrFloat(data.doc, affector_node, "speed_u", texture_uv_affector->u_speed());
						PutAttrFloat(data.doc, affector_node, "speed_v", texture_uv_affector->v_speed());
						PutAttrInt(data.doc, affector_node, "coord_idx", texture_uv_affector->coord_idx());
						if (texture_uv_affector->delay() > 0.f)
							PutAttrFloat(data.doc, affector_node, "delay", texture_uv_affector->delay());
						if (texture_uv_affector->period() > 0.f)
							PutAttrFloat(data.doc, affector_node, "period", texture_uv_affector->period());
					}
					break;
					
				case AFFECTOR_ATLAS_ANIM:
					{
						AtlasAnimAffector* atlas_affector = static_cast<AtlasAnimAffector*>(creator->affectors[i]);
						affector_node = CreateNode(data.doc, "atlas_anim_affector");
						PutAttrStr(data.doc, affector_node, "atlas", GetFileName(atlas_affector->atlas_res()));
						PutAttrStr(data.doc, affector_node, "prefix", atlas_affector->atlas_prefix());
						PutAttrInt(data.doc, affector_node, "coord_idx", atlas_affector->coord_idx());
						PutAttrFloat(data.doc, affector_node, "interval", atlas_affector->interval());
						PutAttrBool(data.doc, affector_node, "loop", atlas_affector->loop());
						if (atlas_affector->delay() > 0.f)
							PutAttrFloat(data.doc, affector_node, "delay", atlas_affector->delay());
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
	}
	
	ParticleSystemCreator* LoadParticleSystemCreatorByScriptFile(const std::string& path)
	{
		std::string absolute_dir;
		size_t pos = path.rfind('/');
		if (pos == std::string::npos)
			pos = path.rfind('\\');
		if (pos != std::string::npos)
			absolute_dir = path.substr(0, pos) + "/";
		
		XmlParseData parse_data;
		ParseFile(path, parse_data);

		xml_node<>* node = parse_data.doc.first_node("particle_system");
		if (node)
			return LoadParticleSystemCreator(node, absolute_dir);

		return NULL;
	}

	bool SaveParticleSystemToScriptByCreator(const ParticleSystemCreator* creator, const std::string& path)
	{
		ASSERT(creator);
		
		XmlCreateData data;
		SaveParticleSystem(creator, data);
		
		return SaveFile(path, data);
	}
	
	void LoadParticleSystemCreatorByScriptFile(const std::string& path, std::vector<ParticleSystemCreator*>& out_creators)
	{
		ASSERT(out_creators.empty());
		
		std::string absolute_dir;
		size_t pos = path.rfind('/');
		if (pos == std::string::npos)
			pos = path.rfind('\\');
		if (pos != std::string::npos)
			absolute_dir = path.substr(0, pos) + "/";
		
		XmlParseData parse_data;
		ParseFile(path, parse_data);
		
		ParticleSystemCreator* creator;
		
		xml_node<>* node = parse_data.doc.first_node("particle_system");
		while (node)
		{
			creator = LoadParticleSystemCreator(node, absolute_dir);
			if (creator)
				out_creators.push_back(creator);
			
			node = node->next_sibling("particle_system");
		}
	}
	
	bool SaveParticleSystemToScriptByCreator(const std::vector<ParticleSystemCreator*>& creators, const std::string& path)
	{
		ASSERT(!creators.empty());

		XmlCreateData data;
		for (int i = 0; i < creators.size(); ++i)
		  SaveParticleSystem(creators[i], data);

		return SaveFile(path, data);
	}
	
}
