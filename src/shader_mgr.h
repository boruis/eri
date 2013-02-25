//
//  shader_mgr.h
//  eri
//
//  Created by exe on 10/8/12.
//
//

#ifndef ERI_SHADER_MGR_H
#define ERI_SHADER_MGR_H

#include <vector>
#include <map>

namespace ERI
{

enum UNIFORM_INDEX
{
	UNIFORM_MODEL_VIEW_PROJ_MATRIX,
	UNIFORM_TEX_ENABLE,
	UNIFORM_TEX0,
	UNIFORM_TEX1,
	UNIFORM_TEX_COORD1,
	UNIFORM_TEX_MATRIX_ENABLE,
	UNIFORM_TEX_MATRIX0,
	UNIFORM_TEX_MATRIX1,
	UNIFORM_MAX
};

enum ATTRIB_INDEX
{
	ATTRIB_VERTEX,
	ATTRIB_NORMAL,
	ATTRIB_COLOR,
	ATTRIB_TEXCOORD0,
	ATTRIB_TEXCOORD1,
	ATTRIB_MAX
};

class ShaderProgram
{
public:
	ShaderProgram();
	~ShaderProgram();
	
	bool Construct(const std::string& vertex_shader_path,
				   const std::string& fragment_shader_path);
	
	inline unsigned int program() { return program_; }
	inline const std::vector<int>& uniforms() { return uniforms_; }
	
private:
	unsigned int program_;
	std::vector<int> uniforms_;
};

class ShaderMgr
{
public:
	ShaderMgr();
	~ShaderMgr();
	
	ShaderProgram* Create(const std::string& name,
						  const std::string& vertex_shader_path,
						  const std::string& fragment_shader_path);
	
	ShaderProgram* Get(const std::string& name);
	
	inline ShaderProgram* default_program() { return default_program_; }
	inline void set_default_program(ShaderProgram* program) { default_program_ = program; }
	
private:
	std::map<std::string, ShaderProgram*> program_map_;
	
	ShaderProgram* default_program_;
};

}

#endif
