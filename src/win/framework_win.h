/*
 *  framework_win.h
 *  eri
 *
 *  Created by exe on 09/05/10.
 *  Copyright 2010 cobbler. All rights reserved.
 *
 */

#ifndef ERI_FRAMEWORK_WIN_H
#define ERI_FRAMEWORK_WIN_H

#include <Windows.h>

namespace ERI {

	struct WinInitInfo;

	class FrameworkWin
	{
	public:
		FrameworkWin();
		~FrameworkWin();

		bool Init(HINSTANCE hInstance, const char* title, int width, int height, bool use_depth_buffer = true);
		void Release();

		void Run();

		float PreUpdate();	// return delta_time
		void PostUpdate();

		bool IsVisible();

		inline bool is_running() { return is_running_; }

	private:
		bool RegisterWindowClass();
		bool ChangeScreenResolution(int width, int height, int bits_per_pixel);
		bool ConstructWindow();
		void DestructWindow();

		WinInitInfo*	win_init_info_;
		bool			is_running_;

		static const double		clock_per_sec_invert_;
	};

}

#endif // ERI_FRAMEWORK_WIN_H