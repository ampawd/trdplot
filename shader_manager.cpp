#include "shader_manager.h"

using namespace app_nmsp;

ShaderManager::ShaderManager() : currentShaderProgram(0)
{
}

GLuint ShaderManager::buildProgram(const Shader& vshaderInstance, const Shader& fhaderInstance, GLuint* program)
{
	if (!compileShader(vshaderInstance))
	{
		showShaderInfoLog(vshaderInstance);
		return pvr::Result::Enum::InvalidData;
	}
	if (!compileShader(fhaderInstance))
	{
		showShaderInfoLog(fhaderInstance);
		return pvr::Result::Enum::InvalidData;
	}

	*program = glCreateProgram();
	glAttachShader(*program, vshaderInstance.getID());
	glAttachShader(*program, fhaderInstance.getID());

	glLinkProgram(*program);
	GLint linked;
	glGetProgramiv(*program, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		showProgramInfoLog(*program);
		return pvr::Result::Enum::InvalidData;
	}

	glDeleteShader(vshaderInstance.getID());
	glDeleteShader(fhaderInstance.getID());

	return pvr::Result::Enum::Success;
}

void ShaderManager::use(GLuint program)
{
	glUseProgram(program);
	currentShaderProgram = program;
}

GLuint ShaderManager::getUsingProgram() const
{
	return currentShaderProgram;
}

const std::string& app_nmsp::ShaderManager::getErrorMessage() const
{
	return errorMessage;
}

GLint ShaderManager::compileShader(const Shader& shaderInstance) const
{
	GLint compiled;
	std::string csource = shaderInstance.getSource();
	const char* source = csource.c_str();
	glShaderSource(shaderInstance.getID(), 1, &source, NULL);
	glCompileShader(shaderInstance.getID());
	glGetShaderiv(shaderInstance.getID(), GL_COMPILE_STATUS, &compiled);
	return compiled;
}

void ShaderManager::showShaderInfoLog(const Shader& shaderInstance)
{
	char msg[1024];
	glGetShaderInfoLog(shaderInstance.getID(), 512, NULL, msg);
	errorMessage = shaderInstance.getType() == GL_VERTEX_SHADER ?
		"Vertex Shader " : "Fragment shader ";

	errorMessage += msg;
}

void ShaderManager::showProgramInfoLog(GLuint program)
{
	char msg[1024];
	glGetProgramInfoLog(program, 512, NULL, msg);
	errorMessage = "Shader program linkling faild: ";

	errorMessage += msg;
}