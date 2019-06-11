#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <vector>

namespace gl
{
	class Transformable
	{
	private:
		static glm::mat4 base;
		static std::vector<Transformable*> transformables;
	protected:
		glm::fmat4 matrix;

		glm::fvec2 position = glm::fvec2(0.f, 0.f);
		glm::fvec2 size = glm::fvec2(1.f, 1.f);
		glm::fvec2 origin = glm::fvec2(0.f, 0.f);
		float depth = 0.f;
		float rotation = 0.f;

	public:
		Transformable()
		{
			transformables.push_back(this);
		}

		~Transformable()
		{
			std::remove(transformables.begin(), transformables.end(), this);
		}

		inline void SetPosition(const glm::vec2 value)
		{
			position = value;
		}
		inline void SetDepth(const float value)
		{
			depth = value;
		}
		inline void SetRotation(const float value)
		{
			rotation = value;
		}
		inline void SetSize(const glm::vec2 value)
		{
			size = value;
		}
		inline void SetOrigin(const glm::vec2 value)
		{
			origin = value;
		}

		inline void Update()
		{
			matrix = glm::translate(base, glm::vec3(position,depth));
			matrix = glm::scale(matrix, glm::vec3(size, 0.0));
			matrix = glm::translate(matrix, glm::vec3(-origin/size, 0.0));
			matrix = glm::rotate(matrix,rotation, glm::vec3(0,0,1));
			matrix = glm::translate(matrix, glm::vec3(-origin / size, 0.0));
		}

		static inline void Ortho(const float left,const float right,const float bottom,const float top,const float near,const float far)
		{
			base = glm::ortho(left, right, bottom, top, near, far);
			for (auto& t : transformables)
			{
				t->Update();
			}
		}
	};
	glm::mat4 Transformable::base = glm::mat4(1);
	std::vector<Transformable*> Transformable::transformables;
}