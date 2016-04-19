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

	InputMgr::InputMgr() : handler_(NULL), global_handler_(NULL)
	{
	}
	
	InputMgr::~InputMgr()
	{
	}

	void InputMgr::Press(const InputEvent& event)
	{
#ifdef ERI_RECORD_MULTI_TOUCH
		AddTouch(event);
#endif

		if (handler_) handler_->Press(event);
	}
	
	void InputMgr::Release(const InputEvent& event)
	{
#ifdef ERI_RECORD_MULTI_TOUCH
		RemoveTouch(event);
#endif

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
#ifdef ERI_RECORD_MULTI_TOUCH
		AddTouch(event);
#endif

		if (handler_) handler_->Move(event);
	}
	
	void InputMgr::MultiMove(const InputEvent* events, int num, bool is_start)
	{
#ifdef ERI_RECORD_MULTI_TOUCH
		for (int i = 0; i < num; ++i)
			AddTouch(events[i]);
#endif

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
		if (handler_)
		{
			if (handler_->KeyDown(event))
				return;
		}
		
		if (global_handler_) global_handler_->KeyDown(event);
	}
	
	void InputMgr::KeyUp(const InputKeyEvent& event)
	{
		if (handler_)
		{
			if (handler_->KeyUp(event))
				return;
		}
		
		if (global_handler_) global_handler_->KeyUp(event);
	}
	
	void InputMgr::Char(const InputKeyEvent& event)
	{
		if (handler_) handler_->Char(event);
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
	
	const InputEvent* InputMgr::GetTouch(long long uid) const
	{
#ifdef ERI_RECORD_MULTI_TOUCH
		size_t num = touches_.size();
		for (int i = 0; i < num; ++i)
		{
			if (touches_[i].uid == uid)
				return &touches_[i];
		}
#endif
		return NULL;
	}

	void InputMgr::AddTouch(const InputEvent& event)
	{
#ifdef ERI_RECORD_MULTI_TOUCH
		size_t num = touches_.size();
		for (int i = 0; i < num; ++i)
		{
			if (touches_[i].uid == event.uid)
			{
				touches_[i] = event;
				return;
			}
		}

		touches_.push_back(event);
#endif
	}

	void InputMgr::RemoveTouch(const InputEvent& event)
	{
#ifdef ERI_RECORD_MULTI_TOUCH
		size_t num = touches_.size();
		for (int i = 0; i < num; ++i)
		{
			if (touches_[i].uid == event.uid)
			{
				if (i < (num - 1))
					touches_[i] = touches_[num - 1];
				
				touches_.pop_back();
				break;
			}
		}
#endif
	}

}
