/*
 *  input_mgr.cpp
 *  math_hero
 *
 *  Created by exe on 12/8/09.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#include "pch.h"

#include "input_mgr.h"

namespace ERI {

	InputMgr::InputMgr() : handler_(NULL)
	{
	}
	
	InputMgr::~InputMgr()
	{
	}

	void InputMgr::Press(int screen_x, int screen_y)
	{
		if (handler_) handler_->Press(screen_x, screen_y);
	}
	
	void InputMgr::Release(int screen_x, int screen_y)
	{
		if (handler_) handler_->Release(screen_x, screen_y);
	}
	
	void InputMgr::Click(int screen_x, int screen_y)
	{
		if (handler_) handler_->Click(screen_x, screen_y);
	}
	
	void InputMgr::DoubleClick(int screen_x, int screen_y)
	{
		if (handler_) handler_->DoubleClick(screen_x, screen_y);
	}

	void InputMgr::Move(int screen_x, int screen_y)
	{
		if (handler_) handler_->Move(screen_x, screen_y);
	}
	
	void InputMgr::MultiMove(const Vector2* moves, int num, bool is_start)
	{
		if (handler_) handler_->MultiMove(moves, num, is_start);
	}
	
	void InputMgr::Accelerate(float g_x, float g_y, float g_z)
	{
		if (handler_) handler_->Accelerate(g_x, g_y, g_z);
	}
	
	void InputMgr::Shake()
	{
		if (handler_) handler_->Shake();
	}
	
}
