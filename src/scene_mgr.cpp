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
	
#pragma mark TextureActorGroup
	
	TextureActorGroup::~TextureActorGroup()
	{
		for (size_t i = 0; i < actor_arrays_.size(); ++i)
		{
			if (actor_arrays_[i]) delete actor_arrays_[i];
		}
	}
	
	void TextureActorGroup::Render(Renderer* renderer)
	{
		size_t num = actor_arrays_.size();
		for (size_t i = 0; i < num; ++i)
		{
			if (actor_arrays_[i] != NULL)
			{
				ActorArray& actors = *actor_arrays_[i];
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
		
		int array_idx = -1;
		int texture_id = actor->material().GetSingleTextureId();
		
		std::map<int, int>::iterator it = texture_map_.find(texture_id);
		if (it == texture_map_.end())
		{
			ActorArray* array = new ActorArray;
			
			// find empty space
			size_t i = 0;
			size_t num = actor_arrays_.size();
			for (; i < num; ++i)
			{
				if (actor_arrays_[i] == NULL)
				{
					actor_arrays_[i] = array;
					array_idx = i;
					break;
				}
			}
			if (i == num)
			{
				actor_arrays_.push_back(array);
				array_idx = num;
			}
			
			texture_map_.insert(std::make_pair(texture_id, array_idx));
		}
		else
		{
			array_idx = it->second;
		}
		
		actor_arrays_[array_idx]->push_back(actor);
	}
	
	void TextureActorGroup::RemoveActor(SceneActor* actor)
	{
		ASSERT(actor);
		
		RemoveActorByTextureId(actor, actor->material().GetSingleTextureId());
	}
	
	void TextureActorGroup::AdjustActorMaterial(SceneActor* actor, int original_texture_id)
	{
		RemoveActorByTextureId(actor, original_texture_id);
		AddActor(actor);
	}
	
	bool TextureActorGroup::IsEmpty()
	{
		size_t num = actor_arrays_.size();
		for (size_t i = 0; i < num; ++i)
		{
			if (actor_arrays_[i]->size() > 0)
				return false;
		}
		
		return true;
	}
	
	SceneActor* TextureActorGroup::GetHitActor(const Vector3& pos)
	{
		SceneActor* actor;
		
		size_t num = actor_arrays_.size();
		for (size_t i = 0; i < num; ++i)
		{
			if (actor_arrays_[i] != NULL)
			{
				ActorArray& actors = *actor_arrays_[i];
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
	
	void TextureActorGroup::RemoveActorByTextureId(SceneActor* actor, int texture_id)
	{
		ASSERT(actor);
		
		std::map<int, int>::iterator it = texture_map_.find(texture_id);
		
		ASSERT(it != texture_map_.end());
		
		// find actor
		ActorArray& actors = *actor_arrays_[it->second];
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
			delete actor_arrays_[it->second];
			actor_arrays_[it->second] = NULL;
			texture_map_.erase(it);
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

#pragma mark SortActorGroup
	
	static bool SortCompareSceneActor(SceneActor* actor1, SceneActor* actor2)
	{
		return (actor1->GetViewDepth() < actor2->GetViewDepth());
	}
	
	void SortActorGroup::Render(Renderer* renderer)
	{
		if (is_sort_dirty_)
		{
			std::sort(actors_.begin(), actors_.end(), SortCompareSceneActor);
			is_sort_dirty_ = false;
		}
		
		size_t num = actors_.size();
		for (int i = 0; i < num; ++i)
		{
			actors_[i]->Render(renderer);
		}
	}
	
	void SortActorGroup::AddActor(SceneActor* actor)
	{
		ASSERT(actor);
		
		actors_.push_back(actor);
	}
	
	void SortActorGroup::RemoveActor(SceneActor* actor)
	{
		ASSERT(actor);
		
		int num = actors_.size();
		for (int i = 0; i < num; ++i)
		{
			if (actors_[i] == actor)
			{
				if (i != num - 1)
					actors_[i] = actors_[num - 1];

				actors_.pop_back();
				break;
			}
		}
	}
	
	bool SortActorGroup::IsEmpty()
	{
		return actors_.empty();
	}

	SceneActor* SortActorGroup::GetHitActor(const Vector3& pos)
	{
		SceneActor* actor;
		
		int num = actors_.size();
		for (int i = num - 1; i >= 0; --i)
		{
			actor = actors_[i]->GetHitActor(pos);
			if (actor)
				return actor;
		}
		
		return NULL;
	}
	
#pragma mark SceneLayer
	
	SceneLayer::SceneLayer(int uid, bool is_sort_alpha, bool is_clear_depth) :
		id_(uid),
		is_visible_(true),
		is_sort_alpha_(is_sort_alpha),
		is_clear_depth_(is_clear_depth)
	{
		opaque_actors_ = new TextureActorGroup;
		alpha_test_actors_ = new TextureActorGroup;
		
		if (is_sort_alpha)
			alpha_blend_actors_ = new SortActorGroup;
		else
			alpha_blend_actors_ = new TextureActorGroup;
	}
	
	SceneLayer::~SceneLayer()
	{
		delete opaque_actors_;
		delete alpha_test_actors_;
		delete alpha_blend_actors_;
	}
	
	void SceneLayer::Render(Renderer* renderer)
	{
		if (is_clear_depth_)
			renderer->ClearDepth();
		
		// opaque
		renderer->EnableBlend(false);
		opaque_actors_->Render(renderer);
				
		// alpha test
		renderer->EnableBlend(true);
		renderer->EnableAlphaTest(true);
		alpha_test_actors_->Render(renderer);
		
		// alpha blend
		renderer->EnableAlphaTest(false);
		alpha_blend_actors_->Render(renderer);
	}
	
	void SceneLayer::AddActor(SceneActor* actor)
	{
		switch (actor->opacity_type())
		{
			case OPACITY_OPAQUE:
				opaque_actors_->AddActor(actor);
				break;
			case OPACITY_ALPHA_TEST:
				alpha_test_actors_->AddActor(actor);
				break;
			case OPACITY_ALPHA_BLEND:
				alpha_blend_actors_->AddActor(actor);
				SetSortDirty();
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
				opaque_actors_->RemoveActor(actor);
				break;
			case OPACITY_ALPHA_TEST:
				alpha_test_actors_->RemoveActor(actor);
				break;
			case OPACITY_ALPHA_BLEND:
				alpha_blend_actors_->RemoveActor(actor);
				break;
			default:
				ASSERT(0);
				break;
		}
	}
	
	void SceneLayer::AdjustActorMaterial(SceneActor* actor, int original_texture_id)
	{
		switch (actor->opacity_type())
		{
			case OPACITY_OPAQUE:
				opaque_actors_->AdjustActorMaterial(actor, original_texture_id);
				break;
			case OPACITY_ALPHA_TEST:
				alpha_test_actors_->AdjustActorMaterial(actor, original_texture_id);
				break;
			case OPACITY_ALPHA_BLEND:
				alpha_blend_actors_->AdjustActorMaterial(actor, original_texture_id);
				break;
			default:
				ASSERT(0);
				break;
		}
	}
	
	SceneActor* SceneLayer::GetHitActor(const Vector3& pos)
	{
		SceneActor* actor;

		actor = alpha_blend_actors_->GetHitActor(pos);
		if (actor)
			return actor;

		actor = alpha_test_actors_->GetHitActor(pos);
		if (actor)
			return actor;

		actor = opaque_actors_->GetHitActor(pos);
		if (actor)
			return actor;
		
		return NULL;
	}
	
	void SceneLayer::SetSortAlpha(bool sort_alpha)
	{
		if (is_sort_alpha_ == sort_alpha)
			return;
		
		ASSERT(alpha_blend_actors_);
		ASSERT(alpha_blend_actors_->IsEmpty());
		
		delete alpha_blend_actors_;
		
		is_sort_alpha_ = sort_alpha;
		
		if (is_sort_alpha_)
			alpha_blend_actors_ = new SortActorGroup;
		else
			alpha_blend_actors_ = new TextureActorGroup;
	}
	
	void SceneLayer::SetSortDirty()
	{
		if (is_sort_alpha_)
		{
			reinterpret_cast<SortActorGroup*>(alpha_blend_actors_)->set_sort_dirty();
		}
	}

#pragma mark SceneMgr

	SceneMgr::SceneMgr() : current_cam_(NULL)
	{
		CreateLayer(1); // default layer
	}
	
	SceneMgr::~SceneMgr()
	{
		ClearLayer();
	}
	
	void SceneMgr::CreateLayer(int num)
	{
		ASSERT(num > 0);
		
		ClearLayer();
		
		for (int i = 0; i < num; ++i)
		{
			layers_.push_back(new SceneLayer(i, false, false));
		}
	}
	
	void SceneMgr::SetLayerVisible(int layer_id, bool visible)
	{
		ASSERT(layer_id < static_cast<int>(layers_.size()));

		layers_[layer_id]->set_is_visible(visible);
	}
	
	void SceneMgr::SetLayerClearDepth(int layer_id, bool clear_depth)
	{
		ASSERT(layer_id < static_cast<int>(layers_.size()));
		
		layers_[layer_id]->set_is_clear_depth(clear_depth);
	}
	
	void SceneMgr::SetLayerSortAlpha(int layer_id, bool sort_alpha)
	{
		ASSERT(layer_id < static_cast<int>(layers_.size()));
		
		layers_[layer_id]->SetSortAlpha(sort_alpha);
	}
	
	void SceneMgr::ClearLayer()
	{
		for (int i = 0; i < layers_.size(); ++i)
		{
			delete layers_[i];
		}
		layers_.clear();
	}
	
	void SceneMgr::AddActor(SceneActor* actor, int layer_id /* = 0*/)
	{
		ASSERT(actor);
		ASSERT(layer_id < static_cast<int>(layers_.size()));
		
		layers_[layer_id]->AddActor(actor);
		actor->layer_ = layers_[layer_id];
	}
	
	void SceneMgr::RemoveActor(SceneActor* actor, int layer_id)
	{
		ASSERT(actor);
		ASSERT(layer_id < static_cast<int>(layers_.size()));
		
		layers_[layer_id]->RemoveActor(actor);
		actor->layer_ = NULL;
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
			if (layers_[i]->is_visible())
			{
				layers_[i]->Render(renderer);
			}
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
