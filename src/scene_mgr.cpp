/*
 *  scene_mgr.cpp
 *  eri
 *
 *  Created by exe on 11/29/09.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#include "pch.h"

#include "scene_mgr.h"
#include "scene_actor.h"
#include "renderer.h"
#include "texture_mgr.h"
#include "root.h"

namespace ERI {
	
	TextureActorGroup::~TextureActorGroup()
	{
		for (size_t i = 0; i < groups.size(); ++i)
		{
			if (groups[i]) delete groups[i];
		}
	}
	
	void TextureActorGroup::Render(Renderer* renderer)
	{
		size_t num = groups.size();
		for (size_t i = 0; i < num; ++i)
		{
			if (groups[i] != NULL)
			{
				std::vector<SceneActor*>& actors = groups[i]->actors;
				size_t num = actors.size();
				for (size_t j = 0; j < num; ++j)
				{
					actors[j]->Render(renderer);
				}
			}
		}
	}
	
	void TextureActorGroup::AddActor(SceneActor* actor)
	{
		ASSERT(actor);
		
		int group_idx = -1;
		int texture_id = actor->material().texture ? actor->material().texture->id : 0;
		
		std::map<int, int>::iterator it = texture_map.find(texture_id);
		if (it == texture_map.end())
		{
			ActorGroup* group = new ActorGroup;
			
			// find empty space
			size_t i = 0;
			size_t num = groups.size();
			for (; i < num; ++i)
			{
				if (groups[i] == NULL)
				{
					groups[i] = group;
					group_idx = i;
					break;
				}
			}
			if (i == num)
			{
				groups.push_back(group);
				group_idx = num;
			}
			
			texture_map.insert(std::make_pair(texture_id, group_idx));
		}
		else
		{
			group_idx = it->second;
		}
		
		groups[group_idx]->actors.push_back(actor);
	}
	
	void TextureActorGroup::RemoveActor(SceneActor* actor)
	{
		int texture_id = actor->material().texture ? actor->material().texture->id : 0;
		
		std::map<int, int>::iterator it = texture_map.find(texture_id);
		
		ASSERT(it != texture_map.end());
		
		// find actor
		std::vector<SceneActor*>& actors = groups[it->second]->actors;
		size_t i = 0;
		size_t num = actors.size();
		for (; i < num; ++i)
		{
			if (actors[i] == actor)
				break;
		}
		
		ASSERT(i < num);
		
		if (num <= 1) // no more actor, clear group
		{
			delete groups[it->second];
			groups[it->second] = NULL;
			texture_map.erase(it);
		}
		else
		{
			if (i < num - 1) // copy final actor to removed empty space
			{
				actors[i] = actors[num - 1];
			}
			actors.pop_back();
		}
	}
	
	SceneActor* TextureActorGroup::GetHitActor(const Vector3& pos)
	{
		SceneActor* actor;
		
		size_t num = groups.size();
		for (size_t i = 0; i < num; ++i)
		{
			if (groups[i] != NULL)
			{
				std::vector<SceneActor*>& actors = groups[i]->actors;
				size_t num = actors.size();
				for (size_t j = 0; j < num; ++j)
				{
					actor = actors[j]->GetHitActor(pos);
					if (actor)
						return actor;
				}
			}
		}
		
		return NULL;
	}
	
	
	void SceneLayer::Render(Renderer* renderer)
	{
		// opaque
		renderer->EnableBlend(false);
		opaque_actors.Render(renderer);
				
		// alpha test
		renderer->EnableBlend(true);
		renderer->EnableAlphaTest(true);
		alpha_test_actors.Render(renderer);
		
		// alpha blend
		renderer->EnableAlphaTest(false);
		alpha_blend_actors.Render(renderer);
	}
	
	void SceneLayer::AddActor(SceneActor* actor)
	{
		switch (actor->opacity_type())
		{
			case OPACITY_OPAQUE:
				opaque_actors.AddActor(actor);
				break;
			case OPACITY_ALPHA_TEST:
				alpha_test_actors.AddActor(actor);
				break;
			case OPACITY_ALPHA_BLEND:
				alpha_blend_actors.AddActor(actor);
				break;
			default:
				ASSERT(0);
				break;
		}
	}
	
	void SceneLayer::RemoveActor(SceneActor* actor)
	{
		switch (actor->opacity_type())
		{
			case OPACITY_OPAQUE:
				opaque_actors.RemoveActor(actor);
				break;
			case OPACITY_ALPHA_TEST:
				alpha_test_actors.RemoveActor(actor);
				break;
			case OPACITY_ALPHA_BLEND:
				alpha_blend_actors.RemoveActor(actor);
				break;
			default:
				ASSERT(0);
				break;
		}
	}
	
	SceneActor* SceneLayer::GetHitActor(const Vector3& pos)
	{
		SceneActor* actor;

		actor = alpha_blend_actors.GetHitActor(pos);
		if (actor)
			return actor;

		actor = alpha_test_actors.GetHitActor(pos);
		if (actor)
			return actor;

		actor = opaque_actors.GetHitActor(pos);
		if (actor)
			return actor;
		
		return NULL;
	}
	

	SceneMgr::SceneMgr() : current_cam_(NULL)
	{
		AddLayer(); // default layer
	}
	
	SceneMgr::~SceneMgr()
	{
		for (size_t i = layers_.size() - 1; i >= 0; --i)
		{
			delete layers_[i];
		}
	}
	
	int SceneMgr::AddLayer()
	{
		layers_.push_back(new SceneLayer);
		return (layers_.size() - 1);
	}
	
	void SceneMgr::AddActor(SceneActor* actor, int layer_id /* = 0*/)
	{
		ASSERT(actor);
		ASSERT(layer_id < static_cast<int>(layers_.size()));
		
		layers_[layer_id]->AddActor(actor);
	}
	
	void SceneMgr::RemoveActor(SceneActor* actor, int layer_id)
	{
		ASSERT(actor);
		ASSERT(layer_id < static_cast<int>(layers_.size()));
		
		layers_[layer_id]->RemoveActor(actor);
	}
	
	void SceneMgr::Render(Renderer* renderer)
	{
		if (current_cam_)
		{
			if (current_cam_->is_projection_modified())
				current_cam_->UpdateProjectionMatrix();
			
			if (current_cam_->is_view_modified())
				current_cam_->UpdateViewMatrix();
		}
		
		for (size_t i = 0; i < layers_.size(); ++i)
		{
			layers_[i]->Render(renderer);
		}
	}

	Vector3 SceneMgr::ScreenToWorldPos(int screen_x, int screen_y)
	{
		// TODO: perspective?
		
		Vector3 world_pos;
		
		Vector2 cam_pos;
		float cam_zoom = 1.0f;
		if (current_cam_)
		{
			cam_pos = current_cam_->GetPos();
			cam_zoom = current_cam_->ortho_zoom();
		}
		
		world_pos.x = (screen_x - Root::Ins().renderer()->width() / 2) / cam_zoom + cam_pos.x;
		world_pos.y = (screen_y - Root::Ins().renderer()->height() / 2) / cam_zoom + cam_pos.y;
		
		return world_pos;
	}

	SceneActor* SceneMgr::GetHitActor(const Vector3& pos)
	{
		// TODO: 3D handle?
		
		SceneActor* actor;
		for (int i = layers_.size() - 1; i >= 0; --i)
		{
			actor = layers_[i]->GetHitActor(pos);
			if (actor)
			{
				return actor;
			}
		}
		
		return NULL;
	}
	
	void SceneMgr::OnRenderResize()
	{
		if (current_cam_)
		{
			current_cam_->SetProjectionModified();
		}
		else
		{
			UpdateDefaultProjection();
		}
	}
	
	void SceneMgr::SetCurrentCam(CameraActor* cam)
	{
		if (current_cam_ == cam)
			return;
		
		current_cam_ = cam;
		
		if (current_cam_)
		{
			current_cam_->SetViewProjectionModified();
		}
		else
		{
			UpdateDefaultProjection();
			UpdateDefaultView();
		}
	}
	
	void SceneMgr::UpdateDefaultView()
	{
		Root::Ins().renderer()->UpdateView(Vector3(0, 0, 0), Vector3(0, 0, -1), Vector3(0, 1, 0));
	}
	
	void SceneMgr::UpdateDefaultProjection()
	{
		Root::Ins().renderer()->UpdateOrthoProjection(1, -1000, 1000);
	}
	
}
