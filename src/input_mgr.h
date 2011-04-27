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
	
	enum InputKeyCode
	{
		KEY_NONE,
		KEY_DELETE,
		KEY_BACKSPACE,
		KEY_ESCAPE,
		KEY_LEFT,
		KEY_RIGHT,
		KEY_DOWN,
		KEY_UP
	};
	
	enum FunctionKeyFlag
	{
		FUNC_SHIFT	= 0x01,
		FUNC_CTRL	= 0x02,
		FUNC_ALT	= 0x04,
		FUNC_CMD	= 0x08
	};
	
	struct InputEvent
	{
		InputEvent()
			: uid(0), x(0), y(0), dx(0), dy(0), function_key_status(0) {}
		
		InputEvent(unsigned int _uid, int _x, int _y)
			: uid(_uid), x(_x), y(_y), dx(0), dy(0), function_key_status(0) {}
		
		unsigned int	uid;
		int				x, y, dx, dy;
		unsigned int	function_key_status;
	};
	
	struct InputKeyEvent
	{
		InputKeyEvent() : code(KEY_NONE), function_key_status(0) {}
		
		std::string		characters;
		InputKeyCode	code;
		unsigned int	function_key_status;
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
		virtual void RightClick(const InputEvent& event) {}
		virtual void KeyDown(const InputKeyEvent& event) {}
		virtual void KeyUp(const InputKeyEvent& event) {}
		
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
		void RightClick(const InputEvent& event);
		void KeyDown(const InputKeyEvent& event);
		void KeyUp(const InputKeyEvent& event);

		void Accelerate(const Vector3& g);
		void Shake();
		
		inline void set_handler(InputHandler* handler) { handler_ = handler; }
		
	private:
		InputHandler*	handler_;
	};

}

#endif // ERI_INPUT_MGR_H
