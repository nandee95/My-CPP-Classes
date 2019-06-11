#pragma once

#include "Transformable.hpp"
#include "VertexArray.hpp"
#include "Drawable.hpp"
#include "Shader.hpp"

class Quad : public gl::Transformable, protected gl::VertexArray, public gl::Drawable
{
public:
	Quad()
	{
		const GLfloat vertices[] = {
			0.0,0.0,
			0.0,1.0,
			1.0,1.0,
			1.0,0.0
		};
		this->FillBuffer(vertices, sizeof(vertices));
		this->AddAttribPointer(0, 2, 0);//vertices
		this->AddAttribPointer(1, 2, 2);//uv
	}

	virtual void Draw(gl::Uniform& uniform) const
	{
		uniform.SetMat4f(matrix);
		DrawArray(GL_TRIANGLE_STRIP);
	}
};