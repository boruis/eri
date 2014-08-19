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
			color = Color::WHITE;
		}
		
		Vector2	pos;
		Vector2	velocity;
		Vector2	size;
		Vector2 scale;
		float	rotate_angle;
		float	rotate_speed;
		Color	color;
		int		color_interval;
		Vector2 uv_start[2];
		
		// life
		
		float	life;
		float	lived_time;
		float	lived_percent;
		bool	in_use;
		
		struct AffectorVars
		{
			AffectorVars() : delay_timer(0.f), period_timer(-1.f) {}
			float delay_timer, period_timer;
		};
		
		std::vector<AffectorVars> affector_vars;
	};
	
#pragma mark Emitter
	
	enum EmitterType
	{
		EMITTER_BOX = 0,
		EMITTER_CIRCLE,
		EMITTER_END
	};
	
	class BaseEmitter
	{
	public:
		BaseEmitter(EmitterType type, float rate, float angle_min, float angle_max);
		virtual ~BaseEmitter();
		
		void SetRate(float rate);
		
		void Restart();
		bool CheckIsTimeToEmit(float delta_time, int& out_emit_num);
		
		virtual void GetEmitPosAngle(Vector2& out_pos, float& out_angle) const = 0;
		
		virtual BaseEmitter* Clone() = 0;
		
		inline EmitterType type() const { return type_; }
		inline float rate() const { return rate_; }
		
		inline float angle_min() const { return angle_min_; }
		inline void set_angle_min(float angle_min) { angle_min_ = angle_min; }
		
		inline float angle_max() const { return angle_max_; }
		inline void set_angle_max(float angle_max) { angle_max_ = angle_max; }
		
		inline const Vector2& offset() const { return offset_; }
		inline void set_offset(const Vector2& offset) { offset_ = offset; }

		inline bool angle_base_from_center() const { return angle_base_from_center_; }
		inline void set_angle_base_from_center(bool base_from_center) { angle_base_from_center_ = base_from_center; }

		inline bool align_angle() const { return align_angle_; }
		inline void set_align_angle(bool align_angle) { align_angle_ = align_angle; }

	private:
		EmitterType type_;
    
		float	rate_;
		float	angle_min_, angle_max_;
		
		float	emit_interval_;
		float	emit_remain_time_;
		
		Vector2 offset_;
		
		bool angle_base_from_center_;
		bool align_angle_;
	};
	
	class BoxEmitter : public BaseEmitter
	{
	public:
		BoxEmitter(const Vector2& half_size, float rate, float angle_min, float angle_max);
		virtual ~BoxEmitter();
		
		virtual BaseEmitter* Clone();

		virtual void GetEmitPosAngle(Vector2& out_pos, float& out_angle) const;

		inline const Vector2& half_size() { return half_size_; }
		inline void set_half_size(const Vector2& half_size) { half_size_ = half_size; }

		inline float rotate() const { return rotate_; }
		inline void set_rotate(float rotate) { rotate_ = rotate; }
		
	private:
		Vector2	half_size_;
		float rotate_;
	};
	
	class CircleEmitter : public BaseEmitter
	{
	public:
		CircleEmitter(float radius, float rate, float angle_min, float angle_max);
		virtual ~CircleEmitter();
		
		virtual BaseEmitter* Clone();

		virtual void GetEmitPosAngle(Vector2& out_pos, float& out_angle) const;
		
		inline const float radius() { return radius_; }
		inline void set_radius(float radius) { radius_ = radius; }

		inline const float radius_min() { return radius_min_; }
		inline void set_radius_min(float radius_min) { radius_min_ = radius_min; }

	private:
		float	radius_, radius_min_;
	};
	
#pragma mark Affector
	
	enum AffectorType
	{
		AFFECTOR_ROTATE = 0,
		AFFECTOR_FORCE,
		AFFECTOR_ACCELERATION,
		AFFECTOR_SCALE,
		AFFECTOR_COLOR,
		AFFECTOR_COLOR_INTERVAL,
		AFFECTOR_TEXTURE_UV,
		AFFECTOR_END
	};
	
	class BaseAffector
	{
	public:
		BaseAffector(AffectorType type) : type_(type), delay_(0.f), period_(-1.f) {}
		virtual ~BaseAffector() {}
		
		virtual void InitSetup(Particle* p) {}
		virtual void Update(float delta_time, Particle* p) = 0;
		
		virtual BaseAffector* Clone() = 0;
		
		inline AffectorType type() { return type_; }

		inline float delay() { return delay_; }
		inline void set_delay(float delay) { delay_ = delay; }

		inline float period() { return period_; }
		inline void set_period(float period) { period_ = period; }
		
	private:
		AffectorType type_;
		float delay_, period_;
	};
	
	class RotateAffector : public BaseAffector
	{
	public:
		RotateAffector(float speed, float acceleration = 0.0f);
		virtual ~RotateAffector();

		virtual void InitSetup(Particle* p);
		virtual void Update(float delta_time, Particle* p);
		virtual BaseAffector* Clone();
		
		inline float speed() { return speed_; }
		inline void set_speed(float speed) { speed_ = speed; }
		inline float acceleration() { return acceleration_; }
		inline void set_acceleration(float acceleration) { acceleration_ = acceleration; }
		
	private:
		float	speed_, acceleration_;
	};
	
	class ForceAffector : public BaseAffector
	{
	public:
		ForceAffector(const Vector2& acceleration);
		virtual ~ForceAffector();
		
		virtual void Update(float delta_time, Particle* p);
		virtual BaseAffector* Clone();
		
		inline const Vector2& acceleration() { return acceleration_; }
		inline void set_acceleration(const Vector2& acceleration) { acceleration_ = acceleration; }
		
	private:
		Vector2	acceleration_;
	};
	
	class AccelerationAffector : public BaseAffector
	{
	public:
		AccelerationAffector(float acceleration);
		virtual ~AccelerationAffector();
		
		virtual void Update(float delta_time, Particle* p);
		virtual BaseAffector* Clone();
		
		inline float acceleration() { return acceleration_; }
		inline void set_acceleration(float acceleration) { acceleration_ = acceleration; }
		
	private:
		float	acceleration_;
	};
	
	class ScaleAffector : public BaseAffector
	{
	public:
		ScaleAffector(const Vector2& speed);
		virtual ~ScaleAffector();
		
		virtual void Update(float delta_time, Particle* p);
		virtual BaseAffector* Clone();
		
		inline const Vector2& speed() { return speed_; }
		inline void set_speed(const Vector2& speed) { speed_ = speed; }
		
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
		
		virtual BaseAffector* Clone() { return new ColorAffector(start_, end_); }
		
		inline const Color& start() { return start_; }
		inline void set_start(const Color& start) { start_ = start; }
		inline const Color& end() { return end_; }
		inline void set_end(const Color& end) { end_ = end; }
		
	private:
		Color	start_, end_;
	};
	
	class ColorIntervalAffector : public BaseAffector
	{
	public:
		struct ColorInterval
		{
			float	lived;
			Color	color;
		};

		ColorIntervalAffector();
		virtual ~ColorIntervalAffector();
		
		virtual void InitSetup(Particle* p);
		virtual void Update(float delta_time, Particle* p);
		
		virtual BaseAffector* Clone();
		
		void AddInterval(float lived, const Color& color);
		void RemoveInterval(int idx);
		
		inline std::vector<ColorInterval*>& intervals() { return intervals_; }
		
	private:
		std::vector<ColorInterval*>	intervals_;
	};
	
	class TextureUvAffector : public BaseAffector
	{
	public:
		TextureUvAffector(float u_speed, float v_speed, int coord_idx);
		virtual ~TextureUvAffector();
		
		virtual void Update(float delta_time, Particle* p);
		
		virtual BaseAffector* Clone();
		
		inline float u_speed() { return u_speed_; }
		inline void set_u_speed(float speed) { u_speed_ = speed; }
		inline float v_speed() { return v_speed_; }
		inline void set_v_speed(float speed) { v_speed_ = speed; }
		inline int coord_idx() { return coord_idx_; }
		inline void set_coord_idx(int coord_idx) { coord_idx_ = coord_idx; }
		
	private:
		float u_speed_, v_speed_;
		int coord_idx_;
	};
	
#pragma mark ParticleSystem
	
	struct ParticleSystemSetup
	{
		ParticleSystemSetup() :
			is_coord_relative(false),
			life(-1.0f),
			delay(0.0f),
			particle_size(Vector2(1.0f, 1.0f)),
			particle_life_min(1.0f), particle_life_max(1.0f),
			particle_speed_min(0.0f), particle_speed_max(0.0f),
			particle_rotate_min(0.0f), particle_rotate_max(0.0f),
			particle_scale_min(1.0f), particle_scale_max(1.0f)
		{
		}
		
		bool		is_coord_relative;
		
		float		life, delay;
		
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
		
		virtual void AddToScene(int layer_id = 0);
		virtual void RemoveFromScene();
		
		virtual void RemoveChild(SceneActor* actor);
		virtual void RemoveAllChilds();

		void AddChildSystem(ParticleSystem* system);
		
		void RefreshSetup();
		
		void SetEmitter(BaseEmitter* emitter);
		void AddAffector(BaseAffector* affector);
		void ClearAffectors();
		
		void Play();
		bool IsPlaying();
		
		void Update(float delta_time);
		
		void ResetParticles();
		
		void SetTexAreaUV(float start_u, float start_v, float width, float height, int coord_idx = 0);
		
		inline const ParticleSystemSetup* setup_ref() { return setup_ref_; }
		
		inline void set_life(float life) { life_ = life; }
		inline float life() { return life_; }
		
	private:
		void EmitParticle(int num);
		Particle* ObtainParticle();
		
		void CreateBuffer();
		void UpdateBuffer();
		
		const ParticleSystemSetup*	setup_ref_;
		float life_;
		float particle_life_max_;
		
		BaseEmitter*				emitter_;
		std::vector<BaseAffector*>	affectors_;
		
		std::vector<Particle*>		particles_;
		int							first_available_particle_idx_;
		
		vertex_2_pos_tex2_color*		vertices_;
		unsigned short*				indices_;
		
		Vector2		system_scale_;
    
		Vector2		uv_start_[2], uv_size_[2];
		
		float		lived_time_;
		float		delay_timer_;
		bool	emit_before_;
		
		std::vector<ParticleSystem*> child_systems_;
	};
	
#pragma mark ParticleSystemCreator

	struct ParticleMaterialUnit
	{
		ParticleMaterialUnit() :
			filter(FILTER_LINEAR),
			wrap(WRAP_REPEAT),
			env_mode(MODE_MODULATE),
			coord_idx(0)
		{
		}

		std::string     path;
		TextureFilter   filter;
		TextureWrap     wrap;
		TextureEnvMode  env_mode;
		int coord_idx;
	};

	struct ParticleMaterialSetup
	{
		ParticleMaterialSetup() :
			depth_write(false),
			blend_type(BLEND_NORMAL)
		{
			uv_size[0] = uv_size[1] = Vector2::UNIT;
		}

		~ParticleMaterialSetup()
		{
			for (int i = 0; i < units.size(); ++i)
				delete units[i];
		}
		
		std::vector<ParticleMaterialUnit*> units;

		Vector2 uv_start[2], uv_size[2];

		bool              depth_write;
		ActorBlendType blend_type;
	};
	
	struct ParticleSystemCreator
	{
		ParticleSystemCreator() : emitter(NULL) {}
		~ParticleSystemCreator();
		
		ParticleSystemSetup*		setup;
		BaseEmitter*				emitter;
		std::vector<BaseAffector*>	affectors;
		
		ParticleMaterialSetup		material_setup;
		
		ParticleSystem*	Create();
	};
	
#pragma mark script loader function
	
	ParticleSystemCreator* LoadParticleSystemCreatorByScriptFile(const std::string& path);
	bool SaveParticleSystemToScriptByCreator(const ParticleSystemCreator* creator, const std::string& path);

	void LoadParticleSystemCreatorByScriptFile(const std::string& path, std::vector<ParticleSystemCreator*>& out_creators);
	bool SaveParticleSystemToScriptByCreator(const std::vector<ParticleSystemCreator*>& creators, const std::string& path);

}

#endif // ERI_PARTICLE_SYSTEM_H