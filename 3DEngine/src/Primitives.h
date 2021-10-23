#pragma once

// GLEW
#include <glew.h>

// GLFW
#include <glfw3.h>

// Other
#include <vector>
#include "Vertex.h"

class Primitive
{
private:
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;

public:
	// Not really as usefull a class now, Was used primarily in earlier stages of development for easy debugging.
	Primitive() {}
	virtual ~Primitive() {}

	void set(const Vertex* vertices, const unsigned nrOfVertices, const GLuint* indices, const unsigned nrOfIndices)
	{
		for (size_t i = 0; i < nrOfVertices; i++)
		{
			this->vertices.push_back(vertices[i]);
		}
		for (size_t i = 0; i < nrOfIndices; i++)
		{
			this->indices.push_back(indices[i]);
		}
	}

	inline Vertex* getVertices()
	{
		return this->vertices.data();
	}

	inline GLuint* getIndices()
	{
		return this->indices.data();
	}

	inline const unsigned  getNrOfVertices()
	{
		return this->vertices.size();
	}

	inline const unsigned  getNrOfIndices()
	{
		return this->indices.size();
	}

};
// PRIMITIVES USED FOR DEBUGGING PURPOSES
// Triangle
class Triangle : public Primitive
{
public:
	Triangle()
		:Primitive()
	{
		Vertex vertices[] =
		{
			//Position                       //Color                          //Texcoords               //Normals					 // tangent	(placeholder values) //bitangent (placeholder values)
			glm::vec3(0.0f, 0.5f, 0.0f),    glm::vec3(1.0f, 0.0f, 0.0f),     glm::vec2(0.0f, 1.0f),    glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f),
			glm::vec3(-0.5f,-0.5f, 0.0f),    glm::vec3(0.0f, 1.0f, 0.0f),     glm::vec2(0.0f, 0.0f),    glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f),
			glm::vec3(0.5f, -0.5f, 0.0f),   glm::vec3(0.0f, 0.0f, 1.0f),     glm::vec2(1.0f, 0.0f),    glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f),

		};
		unsigned nrOfVertices = sizeof(vertices) / sizeof(Vertex);

		GLuint indices[] =
		{
			0, 1, 2, // tri 1
		};
		unsigned nrOfIndices = sizeof(indices) / sizeof(GLuint);

		this->set(vertices, nrOfVertices, indices, nrOfIndices);
	}
};
// Quad
class Quad : public Primitive
{
public:
	Quad()
		:Primitive()
	{
		Vertex vertices[] =
		{
			//Position                       //Color                          //Texcoords               //Normals					 // tangent	(placeholder values) //bitangent (placeholder values)		
			glm::vec3(-0.5f, 0.5f, 0.0f),    glm::vec3(1.0f, 0.0f, 0.0f),     glm::vec2(0.0f, 1.0f),    glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f),
			glm::vec3(-0.5f,-0.5f, 0.0f),    glm::vec3(0.0f, 1.0f, 0.0f),     glm::vec2(0.0f, 0.0f),    glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f),
			glm::vec3(0.5f, -0.5f, 0.0f),   glm::vec3(0.0f, 0.0f, 1.0f),     glm::vec2(1.0f, 0.0f),    glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f),
			glm::vec3(0.5f, 0.5f, 0.0f),    glm::vec3(1.0f, 1.0f, 0.0f),     glm::vec2(1.0f, 1.0f),    glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f)


		};
		unsigned nrOfVertices = sizeof(vertices) / sizeof(Vertex);

		GLuint indices[] =
		{
			0, 1, 2, // tri 1
			0, 2, 3 // tri 2
		};
		unsigned nrOfIndices = sizeof(indices) / sizeof(GLuint);

		this->set(vertices, nrOfVertices, indices, nrOfIndices);
	}
};
