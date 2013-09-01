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

	void InputMgr::Press(const InputEvent& event)
	{
		if (handler_) handler_->Press(event);
	}
	
	void InputMgr::Release(const InputEvent& event)
	{
		if (handler_) handler_->Release(event);
	}
	
	void InputMgr::Click(const InputEvent& event)
	{
		if (handler_) handler_->Click(event);
	}

	void InputMgr::RightPress(const InputEvent& event)
	{
		if (handler_) handler_->RightPress(event);
	}
	
	void InputMgr::RightRelease(const InputEvent& event)
	{
		if (handler_) handler_->RightRelease(event);
	}
	
	void InputMgr::RightClick(const InputEvent& event)
	{
		if (handler_) handler_->RightClick(event);
	}

	void InputMgr::DoubleClick(const InputEvent& event)
	{
		if (handler_) handler_->DoubleClick(event);
	}

	void InputMgr::Move(const InputEvent& event)
	{
		if (handler_) handler_->Move(event);
	}
	
	void InputMgr::MultiMove(const InputEvent* events, int num, bool is_start)
	{
		if (handler_) handler_->MultiMove(events, num, is_start);
	}

	void InputMgr::OverMove(const InputEvent& event)
	{
		if (handler_) handler_->OverMove(event);
	}

	void InputMgr::Scroll(const InputEvent& event)
	{
		if (handler_) handler_->Scroll(event);
	}

	void InputMgr::KeyDown(const InputKeyEvent& event)
	{
		if (handler_) handler_->KeyDown(event);
	}
	
	void InputMgr::KeyUp(const InputKeyEvent& event)
	{
		if (handler_) handler_->KeyUp(event);
	}
	
	void InputMgr::Accelerate(const Vector3& g)
	{
		if (handler_) handler_->Accelerate(g);
	}
	
	void InputMgr::Shake()
	{
		if (handler_) handler_->Shake();
	}
	
	void InputMgr::JoystickDown(JoystickCode code)
	{
		if (handler_) handler_->JoystickDown(code);
	}
	
	void InputMgr::JoystickUp(JoystickCode code)
	{
		if (handler_) handler_->JoystickUp(code);
	}
	
	void InputMgr::JoystickAxis(JoystickCode code, float x, float y)
	{
		if (handler_) handler_->JoystickAxis(code, x, y);
	}

}
