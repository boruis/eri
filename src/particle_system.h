/*
 *  particle_system.h
 *  eri
 *
 *  Created by seedstudio03 on 2010/10/12.
 *  Copyright 2010 cobbler. All rights reserved.
 *
 */

#ifndef ERI_PARTICLE_SYSTEM_H
#define ERI_PARTICLE_SYSTEM_H

#include "scene_actor.h"

namespace ERI
{
	
#pragma mark Particle
	
	struct Particle
	{
		Particle() { Reset(); }
		
		void Reset()
		{
			in_use = false;
			scale = Vector2(1.0f, 1.0f);
		}
		
		Vector3	pos;
		Vector2	velocity;
		Vector2	size;
		Vector2 scale;
		float	rotate_angle;
		float	rotate_speed;
		Color	color;
		int		color_interval;
		
		// life
		
		float	life;
		float	lived_time;
		float	lived_percent;
		bool	in_use;
	};
	
#pragma mark Emitter
	
	enum EmitterType
	{
		EMITTER_BOX,
		EMITTER_END
	};
	
	class BaseEmitter
	{
	public:
		BaseEmitter(float rate, float angle_min, float angle_max);
		virtual ~BaseEmitter();
		
		bool CheckIsTimeToEmit(float delta_time);
		
		float GetEmitAngle();
		
		virtual void GetEmitPos(Vector3& pos) = 0;
		
		inline float rate() { return rate_; }
	
	private:
		float	rate_;
		float	angle_min_, angle_max_;
		
		float	emit_interval_;
		float	emit_remain_time_;
	};
	
	class BoxEmitter : public BaseEmitter
	{
	public:
		BoxEmitter(Vector3 half_size, float rate, float angle_min, float angle_max);
		virtual ~BoxEmitter();
		
	protected:
		virtual void GetEmitPos(Vector3& pos);
		
	private:
		Vector3	half_size_;
	};
	
#pragma mark Affector
	
	class BaseAffector
	{
	public:
		BaseAffector() {}
		virtual ~BaseAffector() {}
		
		virtual void InitSetup(Particle* p) {}
		virtual void Update(float delta_time, Particle* p) = 0;
	};
	
	class RotateAffector : public BaseAffector
	{
	public:
		RotateAffector(float speed, float acceleration = 0.0f);
		virtual ~RotateAffector();

		virtual void InitSetup(Particle* p);
		virtual void Update(float delta_time, Particle* p);
		
	private:
		float	speed_, acceleration_;
	};
	
	class ForceAffector : public BaseAffector
	{
	public:
		ForceAffector(const Vector2& acceleration);
		virtual ~ForceAffector();
		
		virtual void Update(float delta_time, Particle* p);
		
	private:
		Vector2	acceleration_;
	};
	
	class AccelerationAffector : public BaseAffector
	{
	public:
		AccelerationAffector(float acceleration);
		virtual ~AccelerationAffector();
		
		virtual void Update(float delta_time, Particle* p);
		
	private:
		float	acceleration_;
	};
	
	class ScaleAffector : public BaseAffector
	{
	public:
		ScaleAffector(const Vector2& speed);
		virtual ~ScaleAffector();
		
		virtual void Update(float delta_time, Particle* p);
		
	private:
		Vector2	speed_;
	};
	
	class ColorAffector : public BaseAffector
	{
	public:
		ColorAffector(const Color& start, const Color& end);
		virtual ~ColorAffector();
		
		virtual void InitSetup(Particle* p);
		virtual void Update(float delta_time, Particle* p);
		
	private:
		Color	start_, end_;
	};
	
	class ColorIntervalAffector : public BaseAffector
	{
	public:
		ColorIntervalAffector();
		virtual ~ColorIntervalAffector();
		
		virtual void InitSetup(Particle* p);
		virtual void Update(float delta_time, Particle* p);
		
		void AddInterval(float lived_percent, const Color& color);
		
	private:
		struct ColorInterval
		{
			float	lived_percent;
			Color	color;
		};
		
		std::vector<ColorInterval>	intervals_;
	};
	
	
#pragma mark ParticleSystem
	
	struct ParticleSystemSetup
	{
		ParticleSystemSetup() :
			is_coord_relative(false),
			life(-1.0f),
			particle_size(Vector2(1.0f, 1.0f)),
			particle_life_min(1.0f), particle_life_max(1.0f),
			particle_speed_min(0.0f), particle_speed_max(0.0f),
			particle_rotate_min(0.0f), particle_rotate_max(0.0f),
			particle_scale_min(1.0f), particle_scale_max(1.0f)
		{
		}
		
		bool		is_coord_relative;
		
		float		life;
		
		Vector2		particle_size;
		
		float		particle_life_min, particle_life_max;
		float		particle_speed_min, particle_speed_max;
		float		particle_rotate_min, particle_rotate_max;
		float		particle_scale_min, particle_scale_max;
	};
	
	class ParticleSystem : public SceneActor
	{
	public:
		ParticleSystem(const ParticleSystemSetup* setup_ref);
		~ParticleSystem();
		
		void RefreshSetup();
		
		void SetEmitter(BaseEmitter* emitter);
		void AddAffector(BaseAffector* affector);
		
		void Play();
		bool IsPlaying();
		
		void Update(float delta_time);
		
		void SetTexAreaUV(float start_u, float start_v, float width, float height);
		
		inline const ParticleSystemSetup* setup_ref() { return setup_ref_; }
		
		inline void set_custom_life(float life) { custom_life_ = life; }
		
	private:
		void EmitParticle();
		Particle* ObtainParticle();
		
		void CreateBuffer();
		void UpdateBuffer();
		
		const ParticleSystemSetup*	setup_ref_;
		float						custom_life_;
		
		BaseEmitter*				emitter_;
		std::vector<BaseAffector*>	affectors_;
		
		std::vector<Particle*>		particles_;
		int							first_available_particle_idx_;
		
		vertex_3_pos_tex_color*		vertices_;
		unsigned short*				indices_;
		
		Vector2		uv_start_, uv_size_;
		
		float		lived_time_;
	};
	
#pragma mark script loader function
	
	ParticleSystem* CreateParticleSystemByScriptFile(const std::string& path);

}

#endif // ERI_PARTICLE_SYSTEM_H