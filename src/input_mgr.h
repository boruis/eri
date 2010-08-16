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

namespace ERI {
	
	class Vector2;
	
	class InputHandler
	{
	public:
		virtual void Press(int screen_x, int screen_y) {}
		virtual void Release(int screen_x, int screen_y) {}
		virtual void Click(int screen_x, int screen_y) {}
		virtual void DoubleClick(int screen_x, int screen_y) {}
		virtual void Move(int screen_x, int screen_y) {}
		virtual void MultiMove(const Vector2* moves, int num, bool is_start) {}
		
		virtual void Accelerate(float g_x, float g_y, float g_z) {}
		virtual void Shake() {}
	};

	class InputMgr
	{
	public:
		InputMgr();
		~InputMgr();
		
		void Press(int screen_x, int screen_y);
		void Release(int screen_x, int screen_y);
		void Click(int screen_x, int screen_y);
		void DoubleClick(int screen_x, int screen_y);
		void Move(int screen_x, int screen_y);
		void MultiMove(const Vector2* moves, int num, bool is_start);

		void Accelerate(float g_x, float g_y, float g_z);
		void Shake();
		
		void set_handler(InputHandler* handler) { handler_ = handler; }
		
	private:
		InputHandler*	handler_;
	};

}

#endif // ERI_INPUT_MGR_H
