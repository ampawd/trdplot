#ifndef SHADER_H
#define SHADER_H

#define GLEW_STATIC

#include <string>
#include <fstream>

#include <GLES3/gl3.h>

namespace app_nmsp
{
	class Shader
	{
		public:
			Shader() = delete;
			Shader(const Shader&) = delete;
			Shader(Shader&&) = delete;
			Shader& operator=(const Shader&) = delete;

			explicit Shader(GLint, const std::string&);
			std::string getSource() const;
			GLint getType() const;
			GLuint getID() const;

		private:
			std::string source;
			GLint type;
			GLuint ID;

		protected:
	};

}

#endif // SHADER_H