/*
 *  render_context_win.cpp
 *  eri
 *
 *  Created by exe on 08/29/10.
 *  Copyright 2010 cobbler. All rights reserved.
 *
 */

#include "pch.h"
#include "render_context_win.h"

#include <cstdio>
#include <Windows.h>
#include "GL/glew.h"
//#include "GL/wglew.h"
//#include "GLee.h"

#include "root.h"

namespace ERI {

#pragma mark WinInfo

	struct WinInfo
	{
		WinInfo() : hWnd(0), hDC(0), hRC(0) {}

		HWND	hWnd;
		HDC		hDC;
		HGLRC	hRC;
	};

#pragma mark RenderContextWin

	RenderContextWin::RenderContextWin() : win_info_(NULL)
	{
	}

	RenderContextWin::~RenderContextWin()
	{
		ASSERT(win_info_);

		if (win_info_->hDC != 0)
		{
			wglMakeCurrent (win_info_->hDC, 0);	// Set The Current Active Rendering Context To Zero
			if (win_info_->hRC != 0)
			{
				wglDeleteContext(win_info_->hRC);
			}
			ReleaseDC(win_info_->hWnd, win_info_->hDC);
		}

		delete win_info_;
	}

	bool RenderContextWin::Init(int version)
	{
		if (!win_info_) win_info_ = new WinInfo;

		win_info_->hWnd = static_cast<HWND>(Root::Ins().window_handle());

		ASSERT(win_info_->hWnd);

		// Grab A Device Context For This Window
		win_info_->hDC = GetDC(win_info_->hWnd);
		if (win_info_->hDC == 0)
		{
			return false;
		}

		unsigned char bits_per_pixel = 32;

		PIXELFORMATDESCRIPTOR pfd =											// pfd Tells Windows How We Want Things To Be
		{
			sizeof (PIXELFORMATDESCRIPTOR),									// Size Of This Pixel Format Descriptor
			1,																// Version Number
			PFD_DRAW_TO_WINDOW |											// Format Must Support Window
			PFD_SUPPORT_OPENGL |											// Format Must Support OpenGL
			PFD_DOUBLEBUFFER,												// Must Support Double Buffering
			PFD_TYPE_RGBA,													// Request An RGBA Format
			bits_per_pixel,													// Select Our Color Depth
			0, 0, 0, 0, 0, 0,												// Color Bits Ignored
			0,																// No Alpha Buffer
			0,																// Shift Bit Ignored
			0,																// No Accumulation Buffer
			0, 0, 0, 0,														// Accumulation Bits Ignored
			16,																// 16Bit Z-Buffer (Depth Buffer)  
			0,																// No Stencil Buffer
			0,																// No Auxiliary Buffer
			PFD_MAIN_PLANE,													// Main Drawing Layer
			0,																// Reserved
			0, 0, 0															// Layer Masks Ignored
		};

		// Find A Compatible Pixel Format
		GLuint pixel_format = ChoosePixelFormat(win_info_->hDC, &pfd);
		if (pixel_format == 0)
		{
			ReleaseDC(win_info_->hWnd, win_info_->hDC);
			win_info_->hDC = 0;
			return false;
		}

		// Try To Set The Pixel Format
		if (!SetPixelFormat(win_info_->hDC, pixel_format, &pfd))
		{
			ReleaseDC(win_info_->hWnd, win_info_->hDC);
			win_info_->hDC = 0;
			return false;
		}

		// Try To Get A Rendering Context
		win_info_->hRC = wglCreateContext (win_info_->hDC);
		if (win_info_->hRC == 0)
		{
			ReleaseDC(win_info_->hWnd, win_info_->hDC);
			win_info_->hDC = 0;
			return false;
		}

		SetAsCurrent();

		GLenum err = glewInit();
		if (GLEW_OK != err)
		{
			printf("Error: %s\n", glewGetErrorString(err));
		}
		printf("Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

		return true;
	}

	void RenderContextWin::BackingLayer(void* layer)
	{
	}

	void RenderContextWin::SetAsCurrent()
	{
		ASSERT(win_info_);

		// Make The Rendering Context Our Current Rendering Context
		if (!wglMakeCurrent(win_info_->hDC, win_info_->hRC))
		{
			wglDeleteContext(win_info_->hRC);
			win_info_->hRC = 0;
			ReleaseDC (win_info_->hWnd, win_info_->hDC);
			win_info_->hDC = 0;
		}
	}

	void RenderContextWin::Present()
	{
		SwapBuffers(win_info_->hDC);
	}

}
