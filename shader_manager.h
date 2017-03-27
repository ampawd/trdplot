#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

#include "PVRShell/PVRShell.h"

#include "shader.h"

namespace app_nmsp
{
	class ShaderManager
	{
		public:
			ShaderManager(const ShaderManager&) = delete;
			ShaderManager(ShaderManager&&) = delete;
			ShaderManager& operator=(const ShaderManager&) = delete;

			explicit ShaderManager();
			GLuint buildProgram(const Shader&, const Shader&, GLuint*);
			void use(GLuint);
			GLuint getUsingProgram() const;

			const std::string& getErrorMessage() const;

		private:
			GLuint currentShaderProgram;
			std::string errorMessage;
			GLint compileShader(const Shader&) const;
			void showShaderInfoLog(const Shader&);
			void showProgramInfoLog(GLuint);

		protected:

	};
};

#endif	// SHADER_MANAGER_H
