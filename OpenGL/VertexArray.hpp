#pragma once

#include <GL/glew.h>
#include <initializer_list>

namespace gl
{

	template <bool D>
	class VertexArray_
	{
	private:
		GLuint VBO, VAO;
		size_t size;
	public:
		VertexArray_()
		{
			glGenBuffers(1, &this->VBO);
			glGenVertexArrays(1, &this->VAO);
		}
		~VertexArray_()
		{
			glDeleteVertexArrays(1, &VAO);
			glDeleteBuffers(1, &VBO);
		}

		void FillBuffer(const void* vertices, size_t size) noexcept
		{
			glBindVertexArray(this->VAO);

			glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
			glBufferData(GL_ARRAY_BUFFER, size, vertices, D ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
			this->size = size;
		}

		inline void UpdateBuffer(const void* data, const size_t offset, const size_t size) noexcept
		{
			static_assert(D == true, "This method can only be used on a VertexArrayDynamic");
			glBindVertexArray(this->VAO);
			glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
			glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
		}


		void AddAttribPointer(const GLuint position, const GLint length, const size_t shift,const GLenum type = GL_FLOAT,const size_t typeLen = sizeof(GLfloat)) const noexcept
		{
			glVertexAttribPointer(position, length, type, GL_FALSE, length * typeLen, (void*)(shift * typeLen));
			glEnableVertexAttribArray(position);
		}

		inline void DrawArray(GLenum mode, const size_t from, const size_t to) const noexcept
		{
			glBindVertexArray(VAO);
			glDrawArrays(mode, from, to);
		}

		inline void DrawArray(GLenum mode) const noexcept
		{
			glBindVertexArray(VAO);
			glDrawArrays(mode, 0, size);
		}

	};
	typedef VertexArray_<false> VertexArray;
	typedef VertexArray_<true> VertexArrayDynamic;
}