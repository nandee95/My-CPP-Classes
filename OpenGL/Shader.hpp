#pragma once
#include <GL/glew.h>
#include <GL/gl.h>
#include <string>
#include <array>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Exception.hpp"
#include "ScopedPtr.hpp"

namespace gl
{
	class Shader;

	class Uniform
	{
	private:
		const Shader* parent;
		const GLuint id;
	protected:
		friend Shader;
		Uniform(const Shader* parent,const GLuint& location) : parent(parent), id(location)
		{

		}
	public:
		inline void SetInt(const GLint value) const
		{
			glUniform1i(id, value);
		}
		inline void SetFloat(const GLfloat value) const
		{
			glUniform1f(id, value);
		}
		inline void SetVec2i(const glm::ivec2 value) const
		{
			glUniform2i(id, value.x, value.y);
		}
		inline void SetVec2f(const glm::fvec2 value) const
		{
			glUniform2f(id, value.x, value.y);
		}
		inline void SetVec3i(const glm::ivec3 value) const
		{
			glUniform3i(id, value.x, value.y, value.z);
		}
		inline void SetVec3f(const glm::fvec3 value) const
		{
			glUniform3f(id, value.x, value.y, value.z);
		}
		inline void SetMat4f(const glm::fmat4 value) const
		{
			glUniformMatrix4fv(id, 1, GL_FALSE, glm::value_ptr(value));
		}
		inline void SetMat3f(const glm::fmat3 value) const
		{
			glUniformMatrix3fv(id, 1, GL_FALSE, glm::value_ptr(value));
		}
		inline void SetMat2f(const glm::fmat2 value) const
		{
			glUniformMatrix2fv(id, 1, GL_FALSE, glm::value_ptr(value));
		}
	};

	class Shader
	{
	public:
		enum Type : uint8_t
		{
			Vertex, Fragment, Geometry, TYPE_MAX
		};
	protected:
		GLuint id;
		std::string sources[TYPE_MAX];
		std::vector<Uniform> uniforms;
	public:
		Shader()
		{
			id = glCreateProgram();
		}

		~Shader()
		{
			glDeleteProgram(id);
		}

		void LoadShaderFromMemory(const Type type, const std::string source)
		{
			sources[type] = source;
		}

		void Compile()
		{
			//Compile vertex shader
			for (uint8_t i = 0; i < TYPE_MAX; i++)
			{
				if (sources[i].empty()) continue;
				const GLuint shader = glCreateShader(i==Vertex ? GL_VERTEX_SHADER : (i == Fragment ? GL_FRAGMENT_SHADER : GL_GEOMETRY_SHADER));
				glShaderSource(shader, 1, &std::array<const char*, 1> {  this->sources[i].c_str() }[0], nullptr);
				glCompileShader(shader);
				E_RETHROW(CheckShaderCompileErrors(shader, static_cast<Type>(i)));
				glAttachShader(id, shader);
				glDeleteShader(shader);
				sources[i].clear();
			}
			//Link program
			glLinkProgram(id);
			E_RETHROW(CheckLinkerErrors(id));
		}

		inline void Use() const
		{
			glUseProgram(id);
		}

		const GLuint& GetId() const noexcept
		{
			return id;
		}

		Uniform& GetUniform(std::string variable)
		{
			uniforms.push_back(Uniform(this, glGetUniformLocation(id, variable.c_str())));
			return uniforms.back();
		}
	protected:

		static void CheckLinkerErrors(const uint32_t shader)
		{
			int32_t success = 0;
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				ScopedPtr<char> info(new char[1024]);
				glGetProgramInfoLog(shader, 1024, NULL, info.ptr);
				throw Exception(Exception::Shader_LinkProgramFail,std::string(info.ptr));
			}
		}

		static void CheckShaderCompileErrors(const uint32_t& shader, const Type& type)
		{
			int32_t success = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				ScopedPtr<char> info(new char[1024]);
				glGetShaderInfoLog(shader, 1024, NULL, info.ptr);
				throw Exception(type ? Exception::Shader_CompileVertexFail : ( type == Fragment ? Exception::Shader_CompileFragmentFail : Exception::Shader_CompileGeometryFail), std::string(info.ptr));
			}
		}
	};
}