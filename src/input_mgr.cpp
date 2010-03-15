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

	void InputMgr::Move(int screen_x, int screen_y)
	{
		if (handler_) handler_->Move(screen_x, screen_y);
	}
	
}
