/*
 *  framework_win.cpp
 *  eri
 *
 *  Created by exe on 09/05/10.
 *  Copyright 2010 cobbler. All rights reserved.
 *
 */

#include "pch.h"
#include "framework_win.h"

#include <cstdio>
#include <ctime>

#include "root.h"
#include "renderer.h"
#include "input_mgr.h"

namespace ERI {

struct WinInitInfo
{
	HINSTANCE	hInstance;
	char		class_name[32];

	HWND		hWnd;
	char		title[32];
	int			width;
	int			height;
	int			bits_per_pixel;
	bool		is_fullscreen;
	bool		is_visible;
};

static int mouse_down_x = 0;
static int mouse_down_y = 0;
static int right_mouse_down_x = 0;
static int right_mouse_down_y = 0;
static int mouse_move_x = 0;
static int mouse_move_y = 0;

static unsigned int GetFunctionKeyStatus()
{
	unsigned int status = 0;

	if (GetKeyState(VK_SHIFT) & 0xFF00)
		status |= ERI::FUNC_SHIFT;
	if (GetKeyState(VK_CONTROL) & 0xFF00)
		status |= ERI::FUNC_CTRL;
	if (GetKeyState(VK_MENU) & 0xFF00)
		status |= ERI::FUNC_ALT;
	if (GetKeyState(VK_APPS) & 0xFF00)
		status |= ERI::FUNC_CMD;
	
	return status;
}

static ERI::InputKeyCode TranslateKeyCode(WPARAM event_key_code)
{
	ERI::InputKeyCode code = ERI::KEY_NONE;

	switch (event_key_code)
	{
		case VK_BACK:
			code = ERI::KEY_BACKSPACE;
			break;
		case VK_DELETE:
			code = ERI::KEY_DELETE;
			break;
		case VK_ESCAPE:
			code = ERI::KEY_ESCAPE;
			break;
		case VK_LEFT:
			code = ERI::KEY_LEFT;
			break;
		case VK_RIGHT:
			code = ERI::KEY_RIGHT;
			break;
		case VK_DOWN:
			code = ERI::KEY_DOWN;
			break;
		case VK_UP:
			code = ERI::KEY_UP;
			break;
	}
	
	return code;
}

static ERI::InputKeyCode TranslateCharKeyCode(WPARAM event_key_code)
{
	ERI::InputKeyCode code = ERI::KEY_NONE;

	switch (event_key_code)
	{
		case VK_BACK:
			code = ERI::KEY_BACKSPACE;
			break;
		case VK_ESCAPE:
			code = ERI::KEY_ESCAPE;
			break;
	}
	
	return code;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WinInitInfo* win_init_info = reinterpret_cast<WinInitInfo*>(GetWindowLong(hWnd, GWL_USERDATA));

	switch (uMsg)
	{
	case WM_CREATE:
		{
			CREATESTRUCT* creation = (CREATESTRUCT*)(lParam);	// Store Window Structure Pointer
			win_init_info = static_cast<WinInitInfo*>(creation->lpCreateParams);
			SetWindowLong (hWnd, GWL_USERDATA, reinterpret_cast<LONG>(win_init_info));
		}
		return 0;

	case WM_CLOSE:
		PostMessage(win_init_info->hWnd, WM_QUIT, 0, 0);
		return 0;

	case WM_SIZE:
		switch (wParam)
		{
		case SIZE_MINIMIZED:
			win_init_info->is_visible = false;
			return 0;

		case SIZE_MAXIMIZED:
			win_init_info->is_visible = true;
			win_init_info->width = LOWORD(lParam);
			win_init_info->height = HIWORD(lParam);
			if (Root::Ins().renderer())
				Root::Ins().renderer()->Resize(win_init_info->width, win_init_info->height);
			return 0;

		case SIZE_RESTORED:
			win_init_info->is_visible = true;
			win_init_info->width = LOWORD(lParam);
			win_init_info->height = HIWORD(lParam);
			if (Root::Ins().renderer())
				Root::Ins().renderer()->Resize(win_init_info->width, win_init_info->height);
			return 0;
		}
		break;

	case WM_MOUSEMOVE:
		{
			int screen_x = LOWORD(lParam);
			int screen_y = win_init_info->height - HIWORD(lParam);

			InputEvent e;
			e.x = screen_x;
			e.y = screen_y;

			e.dx = screen_x - mouse_move_x;
			e.dy = -(screen_y - mouse_move_y);
			mouse_move_x = screen_x;
			mouse_move_y = screen_y;

			if (wParam & MK_LBUTTON)
			{
				e.function_key_status = GetFunctionKeyStatus();
				Root::Ins().input_mgr()->Move(e);
			}
			else
			{
				Root::Ins().input_mgr()->OverMove(e);
			}

			return 0;
		}
		break;

	case WM_MOUSEWHEEL:
		{
			InputEvent e;
			e.dy = GET_WHEEL_DELTA_WPARAM(wParam) / 50;
			e.function_key_status = GetFunctionKeyStatus();
			Root::Ins().input_mgr()->Scroll(e);
		}
		break;

	case WM_LBUTTONDOWN:
		{
			int screen_x = LOWORD(lParam);
			int screen_y = win_init_info->height - HIWORD(lParam);

			InputEvent e;
			e.x = screen_x;
			e.y = screen_y;
			e.function_key_status = GetFunctionKeyStatus();

			mouse_down_x = screen_x;
			mouse_down_y = screen_y;

			Root::Ins().input_mgr()->Press(e);
			return 0;
		}
		break;

	case WM_RBUTTONDOWN:
		{
			int screen_x = LOWORD(lParam);
			int screen_y = win_init_info->height - HIWORD(lParam);

			right_mouse_down_x = screen_x;
			right_mouse_down_y = screen_y;

			return 0;
		}
		break;

	case WM_LBUTTONUP:
		{
			int screen_x = LOWORD(lParam);
			int screen_y = win_init_info->height - HIWORD(lParam);

			InputEvent e;
			e.x = screen_x;
			e.y = screen_y;
			e.function_key_status = GetFunctionKeyStatus();

			Root::Ins().input_mgr()->Release(e);

			if (ERI::Abs(screen_x - mouse_down_x) < 10 && ERI::Abs(screen_y - mouse_down_y) < 10)
			{
				Root::Ins().input_mgr()->Click(e);
			}

			return 0;
		}
		break;

	case WM_RBUTTONUP:
		{
			int screen_x = LOWORD(lParam);
			int screen_y = win_init_info->height - HIWORD(lParam);

			if (ERI::Abs(screen_x - right_mouse_down_x) < 10 && ERI::Abs(screen_y - right_mouse_down_y) < 10)
			{
				InputEvent e;
				e.x = screen_x;
				e.y = screen_y;
				Root::Ins().input_mgr()->RightClick(e);
			}

			return 0;
		}
		break;

	case WM_LBUTTONDBLCLK:
		{
			int screen_x = LOWORD(lParam);
			int screen_y = win_init_info->height - HIWORD(lParam);

			InputEvent e;
			e.x = screen_x;
			e.y = screen_y;

			Root::Ins().input_mgr()->DoubleClick(e);
			return 0;
		}
		break;

	case WM_KEYDOWN:
		{
			ERI::InputKeyEvent e;
			e.code = TranslateKeyCode(wParam);
			e.function_key_status = GetFunctionKeyStatus();
			if (e.code != ERI::KEY_NONE)
			{
				ERI::Root::Ins().input_mgr()->KeyDown(e);
			}
		}
		break;

	case WM_KEYUP:
		{
			ERI::InputKeyEvent e;
			e.code = TranslateKeyCode(wParam);
			e.function_key_status = GetFunctionKeyStatus();
			if (e.code != ERI::KEY_NONE)
			{
				ERI::Root::Ins().input_mgr()->KeyUp(e);
			}
		}
		break;

	case WM_CHAR:
		{
			ERI::InputKeyEvent e;
			e.code = TranslateCharKeyCode(wParam);
			e.function_key_status = GetFunctionKeyStatus();
			if (e.code == ERI::KEY_NONE)
			{
				char str[2];
				str[1] = 0;
				str[0] = wParam;
				e.characters = str;

				ERI::Root::Ins().input_mgr()->KeyDown(e);
			}
		}
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

const double FrameworkWin::clock_per_sec_invert_ = 1.0 / CLOCKS_PER_SEC;

FrameworkWin::FrameworkWin() : win_init_info_(NULL), is_running_(false)
{
}

FrameworkWin::~FrameworkWin()
{
	Release();
}

bool FrameworkWin::Init(HINSTANCE hInstance, const char* title, int width, int height, bool use_depth_buffer /*= true*/)
{
	if (!win_init_info_) win_init_info_ = new WinInitInfo;

	win_init_info_->hInstance = hInstance;
	sprintf(win_init_info_->class_name, "ERI::FrameworkWin");

	if (!RegisterWindowClass())
	{
		return false;
	}

	strcpy(win_init_info_->title, title);
	win_init_info_->width = width;
	win_init_info_->height = height;
	win_init_info_->bits_per_pixel = 32;
	win_init_info_->is_fullscreen = false;

	if (!ConstructWindow())
	{
		return false;
	}

	ShowWindow(win_init_info_->hWnd, SW_NORMAL);
	win_init_info_->is_visible = true;

	Root::Ins().set_window_handle(win_init_info_->hWnd);

	Root::Ins().Init(use_depth_buffer);
	Root::Ins().renderer()->Resize(win_init_info_->width, win_init_info_->height);

	return true;
}

void FrameworkWin::Release()
{
	if (!win_init_info_) return;

	DestructWindow();

	UnregisterClass(win_init_info_->class_name, win_init_info_->hInstance);

	delete win_init_info_;
	win_init_info_ = NULL;
}

void FrameworkWin::Run()
{
	is_running_ = true;
}

float FrameworkWin::PreUpdate()
{
	static MSG msg;
	while (PeekMessage(&msg, win_init_info_->hWnd, 0, 0, PM_REMOVE) != 0)
	{
		if (msg.message != WM_QUIT)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			is_running_ = false;
		}
	}

	static double prev_time = clock() * clock_per_sec_invert_;
	double now_time = clock() * clock_per_sec_invert_;

	float delta_time = static_cast<float>(now_time - prev_time);

	prev_time = now_time;

	return delta_time;
}

void FrameworkWin::PostUpdate()
{
	// TODO: not visible not disable all update?

	if (!win_init_info_->is_visible)
	{
		WaitMessage ();	// Application Is Minimized Wait For A Message
	}
	else
	{
		Root::Ins().Update();
	}
}

bool FrameworkWin::IsVisible()
{
	ASSERT(win_init_info_);

	return win_init_info_->is_visible;
}

bool FrameworkWin::RegisterWindowClass()
{
	ASSERT(win_init_info_);

	WNDCLASSEX window_class;
	ZeroMemory(&window_class, sizeof(WNDCLASSEX));
	window_class.cbSize			= sizeof(WNDCLASSEX);
	window_class.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraws The Window For Any Movement / Resizing
	window_class.lpfnWndProc	= (WNDPROC)(WindowProc);				// WindowProc Handles Messages
	window_class.hInstance		= win_init_info_->hInstance;
	window_class.hbrBackground	= (HBRUSH)(COLOR_APPWORKSPACE);			// Class Background Brush Color
	window_class.hCursor		= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	window_class.lpszClassName	= win_init_info_->class_name;

	if (!RegisterClassEx(&window_class))
	{
		MessageBox (HWND_DESKTOP, "RegisterClassEx Failed!", "Error", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	return true;
}

bool FrameworkWin::ChangeScreenResolution(int width, int height, int bits_per_pixel)
{
	DEVMODE screen_settings;
	ZeroMemory(&screen_settings, sizeof(DEVMODE));
	screen_settings.dmSize			= sizeof(DEVMODE);
	screen_settings.dmPelsWidth		= width;
	screen_settings.dmPelsHeight	= height;
	screen_settings.dmBitsPerPel	= bits_per_pixel;
	screen_settings.dmFields		= DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	if (ChangeDisplaySettings(&screen_settings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
	{
		MessageBox(HWND_DESKTOP, "Mode Switch Failed.\nRunning In Windowed Mode.", "Error", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}
	return true;
}

bool FrameworkWin::ConstructWindow()
{
	DWORD windowStyle = WS_OVERLAPPEDWINDOW;		// Define Our Window Style
	DWORD windowExtendedStyle = WS_EX_APPWINDOW;	// Define The Window's Extended Style

	RECT windowRect = { 0, 0, win_init_info_->width, win_init_info_->height };	// Define Our Window Coordinates

	if (win_init_info_->is_fullscreen)
	{
		if (!ChangeScreenResolution(win_init_info_->width, win_init_info_->height, win_init_info_->bits_per_pixel))
		{
			win_init_info_->is_fullscreen = FALSE;
		}
		else
		{
			ShowCursor(FALSE);						// Turn Off The Cursor
			windowStyle = WS_POPUP;					// Set The WindowStyle To WS_POPUP (Popup Window)
			windowExtendedStyle |= WS_EX_TOPMOST;	// Set The Extended Window Style To WS_EX_TOPMOST
		}
	}
	else
	{
		AdjustWindowRectEx(&windowRect, windowStyle, 0, windowExtendedStyle);
	}

	win_init_info_->hWnd = CreateWindowEx(windowExtendedStyle,
		win_init_info_->class_name,
		win_init_info_->title,
		windowStyle,
		100, 100,								// Window X,Y Position
		windowRect.right - windowRect.left,		// Window Width
		windowRect.bottom - windowRect.top,		// Window Height
		HWND_DESKTOP,							// Desktop Is Window's Parent
		0,										// No Menu
		win_init_info_->hInstance,
		win_init_info_);

	if (win_init_info_->hWnd == 0)
	{
		MessageBox(HWND_DESKTOP, "Construct Window Failed!", "Error", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	return true;
}

void FrameworkWin::DestructWindow()
{
	if (win_init_info_->hWnd != 0)
	{	
		DestroyWindow(win_init_info_->hWnd);
		win_init_info_->hWnd = 0;
	}

	if (win_init_info_->is_fullscreen)
	{
		ChangeDisplaySettings(NULL, 0);		// Switch Back To Desktop Resolution
		ShowCursor(TRUE);					// Show The Cursor
	}
}

}