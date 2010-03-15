/*
 *  renderer_es2.cpp
 *  eri
 *
 *  Created by exe on 11/28/09.
 *  Copyright 2009 cobbler. All rights reserved.
 *
 */

#include "renderer_es2.h"

#include "render_context_iphone.h"
#include "platform_helper.h"

// uniform index
enum {
    UNIFORM_TRANSLATE,
    NUM_UNIFORMS
};
GLint uniforms[NUM_UNIFORMS];

// attribute index
enum {
    ATTRIB_VERTEX,
    ATTRIB_COLOR,
    NUM_ATTRIBUTES
};

namespace ERI {

	RendererES2::RendererES2() : context_(NULL), default_frame_buffer_(0), color_render_buffer_(0), program_(0)
	{
	}
	
	RendererES2::~RendererES2()
	{
		if (default_frame_buffer_)
		{
			glDeleteFramebuffers(1, &default_frame_buffer_);
		}
		
		if (color_render_buffer_)
		{
			glDeleteRenderbuffers(1, &color_render_buffer_);
		}
		
		if (program_)
		{
			glDeleteProgram(program_);
		}
		
		if (context_) delete context_;
	}
	
	bool RendererES2::Init(bool use_depth_buffer)
	{
		return false;
		
		context_ = new RenderContextIphone;
		if (!context_->Init(2) || !LoadShaders())
		{
			delete context_;
			context_ = NULL;
			return false;
		}
		
		// Create default framebuffer object. The backing will be allocated for the current layer in -resizeFromLayer
		glGenFramebuffers(1, &default_frame_buffer_);
		glGenRenderbuffers(1, &color_render_buffer_);
		glBindFramebuffer(GL_FRAMEBUFFER, default_frame_buffer_);
		glBindRenderbuffer(GL_RENDERBUFFER, color_render_buffer_);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color_render_buffer_);
		
		return true;
	}
	
	void RendererES2::BackingLayer(void* layer)
	{
		// Allocate color buffer backing based on the current layer size
		glBindRenderbuffer(GL_RENDERBUFFER, color_render_buffer_);
		
		context_->BackingLayer(layer);
		
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &backing_width_);
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &backing_height_);
		
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			//NSLog(@"Failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
			//return NO;
			ASSERT(0);
		}
	}
	
	void RendererES2::RenderStart()
	{
	}

	void RendererES2::RenderEnd()
	{
	}
	
	void RendererES2::Render(const RenderData* data)
	{
	}
	
	void RendererES2::Render()
	{
		// Replace the implementation of this method to do your own custom drawing
		
		static const GLfloat squareVertices[] = {
			-0.5f, -0.33f,
			0.5f, -0.33f,
			-0.5f,  0.33f,
			0.5f,  0.33f,
		};
		
		static const GLubyte squareColors[] = {
			255, 255,   0, 255,
			0,   255, 255, 255,
			0,     0,   0,   0,
			255,   0, 255, 255,
		};
		
		static float transY = 0.0f;
		
		// This application only creates a single context which is already set current at this point.
		// This call is redundant, but needed if dealing with multiple contexts.
		context_->SetAsCurrent();
		
		// This application only creates a single default framebuffer which is already bound at this point.
		// This call is redundant, but needed if dealing with multiple framebuffers.
		glBindFramebuffer(GL_FRAMEBUFFER, default_frame_buffer_);
		glViewport(0, 0, backing_width_, backing_height_);
		
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		// Use shader program
		glUseProgram(program_);
		
		// Update uniform value
		glUniform1f(uniforms[UNIFORM_TRANSLATE], (GLfloat)transY);
		transY += 0.075f;	
		
		// Update attribute values
		glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, squareVertices);
		glEnableVertexAttribArray(ATTRIB_VERTEX);
		glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, 1, 0, squareColors);
		glEnableVertexAttribArray(ATTRIB_COLOR);
		
		// Validate program before drawing. This is a good check, but only really necessary in a debug build.
		// DEBUG macro must be defined in your debug configurations if that's not already the case.
#if defined(DEBUG)
		if (!ValidateProgram(program_))
		{
			//NSLog(@"Failed to validate program: %d", program);
			return;
		}
#endif
		
		// Draw
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		
		// This application only creates a single color renderbuffer which is already bound at this point.
		// This call is redundant, but needed if dealing with multiple renderbuffers.
		glBindRenderbuffer(GL_RENDERBUFFER, color_render_buffer_);
		
		context_->Present();
	}
	
	bool RendererES2::LoadShaders()
	{
		GLuint vert_shader, frag_shader;
		
		// create shader program
		program_ = glCreateProgram();
		
		// create and compile vertex shader
		std::string vert_shader_path_name(GetResourcePath());
		vert_shader_path_name += "/Shaders/Shader.vsh";
		
		if (!CompileShader(&vert_shader, GL_VERTEX_SHADER, vert_shader_path_name))
		{
			//NSLog(@"Failed to compile vertex shader");
			return false;
		}
		
		// create and compile fragment shader
		std::string frag_shader_path_name(GetResourcePath());
		frag_shader_path_name += "/Shaders/Shader.fsh";

		if (!CompileShader(&frag_shader, GL_FRAGMENT_SHADER, frag_shader_path_name))
		{
			//NSLog(@"Failed to compile fragment shader");
			return false;
		}
		
		// attach vertex shader to program
		glAttachShader(program_, vert_shader);
		
		// attach fragment shader to program
		glAttachShader(program_, frag_shader);
		
		// bind attribute locations
		// this needs to be done prior to linking
		glBindAttribLocation(program_, ATTRIB_VERTEX, "position");
		glBindAttribLocation(program_, ATTRIB_COLOR, "color");
		
		// link program
		if (!LinkProgram(program_))
		{
			//NSLog(@"Failed to link program: %d", program);
			return false;
		}
		
		// get uniform locations
		uniforms[UNIFORM_TRANSLATE] = glGetUniformLocation(program_, "translate");
		
		// release vertex and fragment shaders
		if (vert_shader)
			glDeleteShader(vert_shader);
		if (frag_shader)
			glDeleteShader(frag_shader);
		
		return true;
	}
	
	bool RendererES2::CompileShader(GLuint* shader, GLenum type, const std::string& file_path)
	{
		GLint status;
		const GLchar *source;
		
		source = (GLchar *)GetStringFileContent(file_path);
		if (!source)
		{
			//NSLog(@"Failed to load vertex shader");
			return false;
		}
		
		*shader = glCreateShader(type);
		glShaderSource(*shader, 1, &source, NULL);
		glCompileShader(*shader);
		
#if defined(DEBUG)
		GLint logLength;
		glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &logLength);
		if (logLength > 0)
		{
			GLchar *log = (GLchar *)malloc(logLength);
			glGetShaderInfoLog(*shader, logLength, &logLength, log);
			//NSLog(@"Shader compile log:\n%s", log);
			free(log);
		}
#endif
		
		glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);
		if (status == 0)
		{
			glDeleteShader(*shader);
			return false;
		}
		
		return true;
	}
			
	bool RendererES2::LinkProgram(GLuint program)
	{
		GLint status;
		
		glLinkProgram(program);
		
#if defined(DEBUG)
		GLint logLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
		if (logLength > 0)
		{
			GLchar *log = (GLchar *)malloc(logLength);
			glGetProgramInfoLog(program, logLength, &logLength, log);
			//NSLog(@"Program link log:\n%s", log);
			free(log);
		}
#endif
		
		glGetProgramiv(program, GL_LINK_STATUS, &status);
		if (status == 0)
			return false;
		
		return true;
	}
	
	bool RendererES2::ValidateProgram(GLuint program)
	{
		GLint logLength, status;
		
		glValidateProgram(program);
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
		if (logLength > 0)
		{
			GLchar *log = (GLchar *)malloc(logLength);
			glGetProgramInfoLog(program, logLength, &logLength, log);
			//NSLog(@"Program validate log:\n%s", log);
			free(log);
		}
		
		glGetProgramiv(program, GL_VALIDATE_STATUS, &status);
		if (status == 0)
			return false;
		
		return true;
	}
	
}