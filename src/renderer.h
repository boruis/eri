/*
 *  renderer.h
 *  eri
 *
 *  Created by exe on 11/28/09.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#ifndef ERI_RENDERER_H
#define ERI_RENDERER_H

#include "math_helper.h"

namespace ERI {
	
	enum ViewOrientation
	{
		PORTRAIT_HOME_BOTTOM,
		PORTRAIT_HOME_TOP,
		LANDSCAPE_HOME_RIGHT,
		LANDSCAPE_HOME_LEFT
	};
	
	enum PixelFormat
	{
		ALPHA,
		RGB,
		RGBA
	};
	
	class RenderData;
	class MaterialData;
	
	class Renderer
	{
	public:
		Renderer() : view_orientation_(PORTRAIT_HOME_BOTTOM) {}
		virtual ~Renderer() {}
		
		virtual bool Init(bool use_depth_buffer) = 0;
		
		virtual void BackingLayer(void* layer) = 0;
		virtual void Resize(int w, int h) = 0;
		
		virtual int width() = 0;
		virtual int height() = 0;
		
		virtual void RenderStart() = 0;
		virtual void RenderEnd() = 0;
		virtual void Render(const RenderData* data) = 0;
		virtual void Render() = 0;
		
		virtual void SaveTransform() = 0;
		virtual void RecoverTransform() = 0;
		
		virtual void EnableDepthTest(bool enable) = 0;
		virtual void EnableDepthWrite(bool enable) = 0;
		virtual void EnableBlend(bool enable) = 0;
		virtual void EnableAlphaTest(bool enable) = 0;
		virtual void EnableMaterial(const MaterialData* data) = 0;
		
		virtual unsigned int GenerateTexture(void* buffer, int width, int height, PixelFormat format) = 0;
		virtual unsigned int BindTexture() = 0;
		
		virtual void SetBgColor(const Color& color) = 0;
		
		virtual void UpdateOrthoProj(float center_x, float center_y, float zoom) = 0;
		virtual void UpdateViewMatrix(const Matrix4& view_matrix) = 0;
		
		virtual void SetViewOrientation(ViewOrientation orientaion) = 0;
		ViewOrientation view_orientation() { return view_orientation_; }
		
	protected:
		ViewOrientation		view_orientation_;
	};

}

#endif // ERI_RENDERER_H
