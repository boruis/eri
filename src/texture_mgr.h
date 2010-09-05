/*
 *  texture_mgr.h
 *  eri
 *
 *  Created by exe on 11/30/09.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#ifndef ERI_TEXTURE_MGR_H
#define ERI_TEXTURE_MGR_H

#include <string>
#include <map>

namespace ERI {
	
	struct Color;
	
	enum TextureFilter
	{
		FILTER_NEAREST,
		FILTER_LINEAR
	};
	
	struct Texture
	{
		Texture(int _id, int _width, int _height);		
		~Texture();
		
		void CopyPixels(const void* _data);
		bool GetPixelColor(Color& out_color, int x, int y) const;
		
		int		id;
		int		width;
		int		height;
		
		void*	data;

		int		bind_frame_buffer;
		
		mutable TextureFilter	filter_min;
		mutable TextureFilter	filter_mag;
	};
	
	class TextureMgr
	{
	public:
		~TextureMgr();
		
		const Texture* GetTexture(const std::string& resource_path, bool keep_texture_data = false);
		const Texture* GetTxtTexture(const std::string& txt, const std::string& font_name, float font_size, float w, float h);
		
		const Texture* GenerateRenderToTexture(int width, int height);
		
		void ReleaseTexture(const Texture* texture);
		
	private:
		std::map<std::string, Texture*>	texture_map_;
	};
	
#pragma mark RenderToTexture
	
	class CameraActor;
	
	class RenderToTexture
	{
	public:
		RenderToTexture(int width, int height, CameraActor* render_cam = NULL);
		~RenderToTexture();
		
		void Init();
		void Release();
		void ProcessRender();
		
		inline const Texture* texture() { return texture_; }
		
	private:
		int				width_, height_;
		const Texture*	texture_;
		
		CameraActor*	render_cam_;
	};
	
}

#endif // ERI_TEXTURE_MGR_H
