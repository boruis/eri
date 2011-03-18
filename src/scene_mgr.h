/*
 *  scene_mgr.h
 *  eri
 *
 *  Created by exe on 11/29/09.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#ifndef ERI_SCENE_MGR_H
#define ERI_SCENE_MGR_H

#include <vector>
#include <map>

#include "observer.h"
#include "math_helper.h"

namespace ERI {
	
	class SceneActor;
	class CameraActor;
	class Renderer;
	
	typedef std::vector<SceneActor*> ActorArray;

	class ActorGroup
	{
	public:
		virtual ~ActorGroup() {}
		
		virtual void Render(Renderer* renderer) = 0;
		virtual void AddActor(SceneActor* actor) = 0;
		virtual void RemoveActor(SceneActor* actor) = 0;
		virtual void AdjustActorMaterial(SceneActor* actor, int original_texture_id) {}
		virtual bool IsEmpty() = 0;

		// TODO: should remove this fuction
		virtual SceneActor* GetHitActor(const Vector3& pos) = 0;
	};
	
	class TextureActorGroup : public ActorGroup
	{
	public:
		~TextureActorGroup();
		
		void Render(Renderer* renderer);
		void AddActor(SceneActor* actor);
		void RemoveActor(SceneActor* actor);
		void AdjustActorMaterial(SceneActor* actor, int original_texture_id);
		bool IsEmpty();
		
		// TODO: should remove this fuction
		SceneActor* GetHitActor(const Vector3& pos);
		
	private:
		void RemoveActorByTextureId(SceneActor* actor, int texture_id);
		
		std::vector<ActorArray*>	actor_arrays_;
		std::map<int, int>			texture_map_;
	};
	
	class SortActorGroup : public ActorGroup
	{
	public:
		SortActorGroup() : is_sort_dirty_(true) {}
		
		void Render(Renderer* renderer);
		void AddActor(SceneActor* actor);
		void RemoveActor(SceneActor* actor);
		bool IsEmpty();
		
		// TODO: should remove this fuction
		SceneActor* GetHitActor(const Vector3& pos);
		
		inline void set_sort_dirty() { is_sort_dirty_ = true; }
		
	private:
		ActorArray	actors_;
		bool		is_sort_dirty_;
	};

	class SceneLayer
	{
	public:
		SceneLayer(int uid, bool is_sort_alpha, bool is_clear_depth);
		~SceneLayer();

		void Render(Renderer* renderer);
		void AddActor(SceneActor* actor);
		void RemoveActor(SceneActor* actor);
		void AdjustActorMaterial(SceneActor* actor, int original_texture_id);
		
		// TODO: should remove this fuction
		SceneActor* GetHitActor(const Vector3& pos);
		
		void SetSortAlpha(bool sort_alpha);
		void SetSortDirty();
		
		inline int id() { return id_; }
		
		inline CameraActor* cam() { return cam_; }
		inline void set_cam(CameraActor* cam) { cam_ = cam; }

		inline bool is_visible() { return is_visible_; }
		inline void set_is_visible(bool visible) { is_visible_ = visible; }
		inline void set_is_clear_depth(bool claear_depth) { is_clear_depth_ = claear_depth; }
		
	private:
		int		id_;
		
		ActorGroup*	opaque_actors_;
		ActorGroup*	alpha_test_actors_;
		ActorGroup*	alpha_blend_actors_;
		
		CameraActor*	cam_;
		
		bool	is_visible_;
		bool	is_sort_alpha_;
		bool	is_clear_depth_;
	};

	class SceneMgr
	{
	public:
		typedef std::pair<int, int> ResizeInfo;
		
		SceneMgr();
		~SceneMgr();
		
		void CreateLayer(int num);
		void SetLayerVisible(int layer_id, bool visible);
		void SetLayerClearDepth(int layer_id, bool clear_depth);
		void SetLayerSortAlpha(int layer_id, bool sort_alpha);
		void SetLayerCam(int layer_id, CameraActor* cam);
		void ClearLayer();

		void AddActor(SceneActor* actor, int layer_id = 0);
		void RemoveActor(SceneActor* actor, int layer_id);
		
		void Render(Renderer* renderer);

		Vector3 ScreenToWorldPos(int screen_x, int screen_y, CameraActor* cam = NULL);
		Vector2 WorldToScreenPos(const Vector3& world_pos, CameraActor* cam = NULL);
		SceneActor* GetHitActor(const Vector3& pos);
		
		void OnViewportResize();
		
		void SetCurrentCam(CameraActor* cam);
	
		inline CameraActor* current_cam() { return current_cam_; }
		
		inline CameraActor* default_cam() { return default_cam_; }
		inline void set_default_cam(CameraActor* cam) { default_cam_ = cam; }
		
		inline Subject<ResizeInfo>& viewport_resize_subject() { return viewport_resize_subject_; }
		
	private:
		void UpdateDefaultView();
		void UpdateDefaultProjection();
		
		std::vector<SceneLayer*>	layers_;
		CameraActor*				current_cam_;
		CameraActor*				default_cam_;
		
		Subject<ResizeInfo>	viewport_resize_subject_;
	};

}

#endif // ERI_SCENE_MGR_H
