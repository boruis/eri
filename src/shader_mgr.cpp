//
//  shader_mgr.cpp
//  eri
//
//  Created by exe on 10/8/12.
//
//

#include "shader_mgr.h"

#ifdef ERI_RENDERER_ES2

#include "renderer_es2.h"
#include "sys_helper.h"
#include "platform_helper.h"

namespace ERI
{

//==============================================================================

static bool CompileShader(GLuint* shader, GLenum type, const char* source)
{
	*shader = glCreateShader(type);
	glShaderSource(*shader, 1, &source, NULL);
	glCompileShader(*shader);
	
#if defined(DEBUG)
	GLint logLength;
	glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 0)
	{
		GLchar* log = (GLchar*)malloc(logLength);
		glGetShaderInfoLog(*shader, logLength, &logLength, log);
		LOGI("Shader compile log:\n%s", log);
		free(log);
	}
#endif
	
	GLint status;
	glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);
	if (status == 0)
	{
		glDeleteShader(*shader);
		return false;
	}
	
	return true;
}

static bool LinkProgram(GLuint program)
{
	glLinkProgram(program);
	
#if defined(DEBUG)
	GLint logLength;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 0)
	{
		GLchar* log = (GLchar*)malloc(logLength);
		glGetProgramInfoLog(program, logLength, &logLength, log);
		LOGI("Program link log:\n%s", log);
		free(log);
	}
#endif
	
	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == 0)
		return false;
	
	return true;
}

static bool ValidateProgram(GLuint program)
{
	glValidateProgram(program);
	
#if defined(DEBUG)
	GLint logLength;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 0)
	{
		GLchar* log = (GLchar*)malloc(logLength);
		glGetProgramInfoLog(program, logLength, &logLength, log);
		LOGI("Program validate log:\n%s", log);
		free(log);
	}
#endif
	
	GLint status;
	glGetProgramiv(program, GL_VALIDATE_STATUS, &status);
	if (status == 0)
		return false;
	
	return true;

}

//==============================================================================

ShaderProgram::ShaderProgram() : program_(0)
{
	uniforms_.resize(UNIFORM_MAX);
}

ShaderProgram::~ShaderProgram()
{
	if (program_) glDeleteProgram(program_);
}

bool ShaderProgram::Construct(const std::string& vertex_shader_path,
							 const std::string& fragment_shader_path)
{	
	GLuint vertex_shader;
	
	std::string shader_code;
	if (!GetFileContentString(std::string(GetResourcePath()) + "/" + vertex_shader_path, shader_code))
	{
		LOGW("Failed to load vertex shader");
		return false;
	}
	
	if (!CompileShader(&vertex_shader, GL_VERTEX_SHADER, shader_code.c_str()))
	{
		LOGW("Failed to compile vertex shader");
		return false;
	}
	
	GLuint fragment_shader;
	
	if (!GetFileContentString(std::string(GetResourcePath()) + "/" + fragment_shader_path, shader_code))
	{
		LOGW("Failed to load fragment shader");
		return false;
	}
	
	if (!CompileShader(&fragment_shader, GL_FRAGMENT_SHADER, shader_code.c_str()))
	{
		LOGW("Failed to compile fragment shader");
		glDeleteShader(vertex_shader);
		return false;
	}
	
	ASSERT(program_ == 0);
	
	// create shader program
	program_ = glCreateProgram();

	// attach vertex shader to program
	glAttachShader(program_, vertex_shader);
	
	// attach fragment shader to program
	glAttachShader(program_, fragment_shader);
	
	// bind attribute locations
	// this needs to be done prior to linking
	glBindAttribLocation(program_, ATTRIB_VERTEX, "a_position");
	glBindAttribLocation(program_, ATTRIB_NORMAL, "a_normal");
	glBindAttribLocation(program_, ATTRIB_COLOR, "a_color");
	glBindAttribLocation(program_, ATTRIB_TEXCOORD0, "a_texcoord0");
	glBindAttribLocation(program_, ATTRIB_TEXCOORD1, "a_texcoord1");
	
	// link program
	if (!LinkProgram(program_))
	{
		LOGW("Failed to link program: %d", program_);
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
		glDeleteProgram(program_);
		program_ = 0;
		return false;
	}
	
	// get uniform locations
	uniforms_[UNIFORM_MODEL_VIEW_PROJ_MATRIX] = glGetUniformLocation(program_, "model_view_proj_matrix");
	uniforms_[UNIFORM_MODEL_VIEW_MATRIX] = glGetUniformLocation(program_, "model_view_matrix");
	uniforms_[UNIFORM_TEX_USE_COORD_INDEX] = glGetUniformLocation(program_, "tex_use_coord_idx");
	uniforms_[UNIFORM_TEX0] = glGetUniformLocation(program_, "tex[0]");
	uniforms_[UNIFORM_TEX1] = glGetUniformLocation(program_, "tex[1]");
	uniforms_[UNIFORM_TEX_MATRIX_ENABLE] = glGetUniformLocation(program_, "tex_matrix_enable");
	uniforms_[UNIFORM_TEX_MATRIX0] = glGetUniformLocation(program_, "tex_matrix[0]");
	uniforms_[UNIFORM_TEX_MATRIX1] = glGetUniformLocation(program_, "tex_matrix[1]");
	uniforms_[UNIFORM_FOG_ENABLE] = glGetUniformLocation(program_, "fog_enable");
	uniforms_[UNIFORM_FOG_MODE] = glGetUniformLocation(program_, "fog_mode");
	uniforms_[UNIFORM_FOG_START] = glGetUniformLocation(program_, "fog_start");
	uniforms_[UNIFORM_FOG_END] = glGetUniformLocation(program_, "fog_end");
	uniforms_[UNIFORM_FOG_DENSITY] = glGetUniformLocation(program_, "fog_density");
	uniforms_[UNIFORM_FOG_COLOR] = glGetUniformLocation(program_, "fog_color");
	
	// release vertex and fragment shaders
	if (vertex_shader)
		glDeleteShader(vertex_shader);
	if (fragment_shader)
		glDeleteShader(fragment_shader);
	
	return true;
}
  
void ShaderProgram::SetCustomUniform(const std::string& name, float value)
{
	int loc = glGetUniformLocation(program_, name.c_str());
	glUniform1f(loc, value);
}
  
void ShaderProgram::SetCustomUniform(const std::string& name, const Vector2& value)
{
	int loc = glGetUniformLocation(program_, name.c_str());
	glUniform2f(loc, value.x, value.y);
}

bool ShaderProgram::Validate()
{
	if (!ValidateProgram(program_))
	{
		LOGW("Failed to validate program: %d", program_);
		return false;
	}

	return true;
}

//==============================================================================

ShaderMgr::ShaderMgr() : default_program_(NULL), current_program_(NULL)
{
}

ShaderMgr::~ShaderMgr()
{
	std::map<std::string, ShaderProgram*>::iterator it = program_map_.begin();
	for (; it != program_map_.end(); ++it)
		delete it->second;
}

ShaderProgram* ShaderMgr::Create(const std::string& name,
								 const std::string& vertex_shader_path,
								 const std::string& fragment_shader_path)
{
	ASSERT(program_map_.find(name) == program_map_.end());
	
	ShaderProgram* program = new ShaderProgram;
	
	if (!program->Construct(vertex_shader_path, fragment_shader_path))
	{
		delete program;
		return NULL;
	}
	
	program_map_[name] = program;
	
	return program;
}

ShaderProgram* ShaderMgr::Get(const std::string& name)
{
	std::map<std::string, ShaderProgram*>::iterator it = program_map_.find(name);
	
	if (it == program_map_.end())
		return NULL;
	
	return it->second;
}
  
void ShaderMgr::Use(ShaderProgram* program)
{
	if (NULL == program)
		program = default_program_;
	
	if (current_program_ != program)
	{
		glUseProgram(program->program());
		current_program_ = program;
	}
	
	ASSERT(current_program_);
}

}

#endif // ERI_RENDERER_ES2
