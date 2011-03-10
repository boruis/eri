/*
 *  input_mgr.h
 *  math_hero
 *
 *  Created by exe on 12/8/09.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#ifndef ERI_INPUT_MGR_H
#define ERI_INPUT_MGR_H

#include <string>

namespace ERI {
	
	struct Vector3;
	
	struct InputEvent
	{
		InputEvent() {}
		InputEvent(unsigned int _uid, int _x, int _y) : uid(_uid), x(_x), y(_y), dx(0), dy(0) {}
		
		unsigned int	uid;
		int				x, y, dx, dy;
	};
	
	enum InputKeyCode
	{
		KEY_NONE,
		KEY_DELETE,
		KEY_ESCAPE
	};
	
	class InputHandler
	{
	public:
		virtual void Press(const InputEvent& event) {}
		virtual void Release(const InputEvent& event) {}
		virtual void Click(const InputEvent& event) {}
		virtual void DoubleClick(const InputEvent& event) {}
		virtual void Move(const InputEvent& event) {}
		virtual void MultiMove(const InputEvent* events, int num, bool is_start) {}
		
		virtual void Scroll(const InputEvent& event) {}
		virtual void OverMove(const InputEvent& event) {}
		virtual void KeyDown(const std::string& characters, InputKeyCode code) {}
		
		virtual void Accelerate(const Vector3& g) {}
		virtual void Shake() {}
	};

	class InputMgr
	{
	public:
		InputMgr();
		~InputMgr();
		
		void Press(const InputEvent& event);
		void Release(const InputEvent& event);
		void Click(const InputEvent& event);
		void DoubleClick(const InputEvent& event);
		void Move(const InputEvent& event);
		void MultiMove(const InputEvent* events, int num, bool is_start);

		void Scroll(const InputEvent& event);
		void OverMove(const InputEvent& event);
		void KeyDown(const std::string& characters, InputKeyCode code = KEY_NONE);

		void Accelerate(const Vector3& g);
		void Shake();
		
		void set_handler(InputHandler* handler) { handler_ = handler; }
		
	private:
		InputHandler*	handler_;
	};

}

#endif // ERI_INPUT_MGR_H
