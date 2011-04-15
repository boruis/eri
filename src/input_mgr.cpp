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
	
	void InputMgr::Scroll(const InputEvent& event)
	{
		if (handler_) handler_->Scroll(event);
	}
	
	void InputMgr::OverMove(const InputEvent& event)
	{
		if (handler_) handler_->OverMove(event);
	}
	
	void InputMgr::RightClick(const InputEvent& event)
	{
		if (handler_) handler_->RightClick(event);
	}

	void InputMgr::KeyDown(const std::string& characters, InputKeyCode code /*= KEY_NONE*/)
	{
		if (handler_) handler_->KeyDown(characters, code);
	}
	
	void InputMgr::KeyUp(const std::string& characters, InputKeyCode code /*= KEY_NONE*/)
	{
		if (handler_) handler_->KeyUp(characters, code);
	}
	
	void InputMgr::Accelerate(const Vector3& g)
	{
		if (handler_) handler_->Accelerate(g);
	}
	
	void InputMgr::Shake()
	{
		if (handler_) handler_->Shake();
	}
	
}
