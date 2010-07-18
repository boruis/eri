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

#include "math_helper.h"

#include <vector>
#include <map>

namespace ERI {
	
	class SceneActor;
	class CameraActor;
	class Renderer;
	
	struct ActorGroup
	{
		std::vector<SceneActor*>	actors;
	};
	
	struct TextureActorGroup
	{
		~TextureActorGroup();
		
		void Render(Renderer* renderer);
		void AddActor(SceneActor* actor);
		void RemoveActor(SceneActor* actor);
		
		SceneActor* GetHitActor(const Vector3& pos);

		std::vector<ActorGroup*>	groups;
		std::map<int, int>			texture_map;
	};

	struct SceneLayer
	{
		void Render(Renderer* renderer);
		void AddActor(SceneActor* actor);
		void RemoveActor(SceneActor* actor);
		
		SceneActor* GetHitActor(const Vector3& pos);
		
		TextureActorGroup	opaque_actors;
		TextureActorGroup	alpha_test_actors;
		TextureActorGroup	alpha_blend_actors;
	};

	class SceneMgr
	{
	public:
		SceneMgr();
		~SceneMgr();
		
		int AddLayer();
		void AddActor(SceneActor* actor, int layer_id = 0);
		void RemoveActor(SceneActor* actor, int layer_id);
		
		void Render(Renderer* renderer);
		
		Vector3 ScreenToWorldPos(int screen_x, int screen_y);
		SceneActor* GetHitActor(const Vector3& pos);
		
		void OnRenderResize();

		void SetCurrentCam(CameraActor* cam);
	
		inline CameraActor* current_cam() { return current_cam_; }
		
	private:
		void UpdateDefaultView();
		void UpdateDefaultProjection();
		
		std::vector<SceneLayer*>	layers_;
		CameraActor*				current_cam_;
	};

}

#endif // ERI_SCENE_MGR_H
