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
#include <vector>

namespace ERI {
	
	struct Vector3;
	
	enum InputKeyCode
	{
		KEY_NONE,
		KEY_DELETE,
		KEY_BACKSPACE,
		KEY_ESCAPE,
		KEY_TAB,
		KEY_LEFT,
		KEY_RIGHT,
		KEY_DOWN,
		KEY_UP,
		KEY_W,
		KEY_S,
		KEY_A,
		KEY_D,
		KEY_SPACE,
		KEY_Q,
		KEY_E,
		KEY_R,
		KEY_P,
		KEY_L,
		KEY_1,
		KEY_2
	};
	
	enum FunctionKeyFlag
	{
		FUNC_SHIFT	= 0x01,
		FUNC_CTRL	= 0x02,
		FUNC_ALT	= 0x04,
		FUNC_CMD	= 0x08
	};
	
	enum JoystickCode
	{
		JOYSTICK_NONE,
		JOYSTICK_DPAD_UP,
		JOYSTICK_DPAD_DOWN,
		JOYSTICK_DPAD_LEFT,
		JOYSTICK_DPAD_RIGHT,
		JOYSTICK_A,
		JOYSTICK_B,
		JOYSTICK_X,
		JOYSTICK_Y,
		JOYSTICK_L1,
		JOYSTICK_R1,
		JOYSTICK_L2,
		JOYSTICK_R2,
		JOYSTICK_BACK,
		JOYSTICK_START,
		JOYSTICK_THUMBL,
		JOYSTICK_THUMBR,
		JOYSTICK_AXISL,
		JOYSTICK_AXISR
	};
	
	struct InputEvent
	{
		InputEvent()
			: uid(-1), x(0), y(0), dx(0), dy(0), function_key_status(0) {}
		
		InputEvent(long long _uid, int _x, int _y)
			: uid(_uid), x(_x), y(_y), dx(0), dy(0), function_key_status(0) {}
		
		long long uid;
		int x, y, dx, dy;
		unsigned int function_key_status;
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
		
		virtual void RightPress(const InputEvent& event) {}
		virtual void RightRelease(const InputEvent& event) {}
		virtual void RightClick(const InputEvent& event) {}

		virtual void DoubleClick(const InputEvent& event) {}

		virtual void Move(const InputEvent& event) {}
		virtual void MultiMove(const InputEvent* events, int num, bool is_start) {}
		virtual void OverMove(const InputEvent& event) {}
		virtual void Scroll(const InputEvent& event) {}
    
		virtual void KeyDown(const InputKeyEvent& event) {}
		virtual void KeyUp(const InputKeyEvent& event) {}
		
		virtual void Accelerate(const Vector3& g) {}
		virtual void Shake() {}
		
		virtual void JoystickDown(JoystickCode code) {}
		virtual void JoystickUp(JoystickCode code) {}
		virtual void JoystickAxis(JoystickCode code, float x, float y) {}
	};

	class InputMgr
	{
	public:
		InputMgr();
		~InputMgr();
		
		void Press(const InputEvent& event);
		void Release(const InputEvent& event);
		void Click(const InputEvent& event);

		void RightPress(const InputEvent& event);
		void RightRelease(const InputEvent& event);
		void RightClick(const InputEvent& event);
    
		void DoubleClick(const InputEvent& event);

		void Move(const InputEvent& event);
		void MultiMove(const InputEvent* events, int num, bool is_start);
		void OverMove(const InputEvent& event);
		void Scroll(const InputEvent& event);
    
		void KeyDown(const InputKeyEvent& event);
		void KeyUp(const InputKeyEvent& event);

		void Accelerate(const Vector3& g);
		void Shake();

		void JoystickDown(JoystickCode code);
		void JoystickUp(JoystickCode code);
		void JoystickAxis(JoystickCode code, float x, float y);

		inline void set_handler(InputHandler* handler) { handler_ = handler; }
		
		inline const std::vector<InputEvent>& touches() { return touches_; }
		
	private:
		void AddTouch(const InputEvent& event);
		void RemoveTouch(const InputEvent& event);
		
		std::vector<InputEvent> touches_;
		
		InputHandler*	handler_;
	};

}

#endif // ERI_INPUT_MGR_H
