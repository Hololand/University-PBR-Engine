#pragma once

// GLEW
#include <glew.h>

// GLFW
#include <glfw3.h>

// MTB
#include <glm.hpp>
#include <vec3.hpp>
#include <vec4.hpp>
#include <mat4x4.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>

// Other
#include<iostream>
#include<string>
#include<fstream>
#include<vector>
#include<sstream>
#include<algorithm>

#include "Vertex.h"

static std::vector<Vertex> loadOBJ(const char* fileName)
{
	std::vector<glm::fvec3> vertex_positions;	// v
	std::vector<glm::fvec2> vertex_texcoords;	// vt
	std::vector<glm::fvec3> vertex_normals;		// vn

	std::vector<GLint> vertex_position_indices; // f
	std::vector<GLint> vertex_texcoord_indices; // f
	std::vector<GLint> vertex_normal_indices;   // f

	std::vector<Vertex> vertices;
	std::stringstream ss;
	std::ifstream in_file(fileName);

	std::string line = "";
	std::string prefix = "";
	glm::vec3 temp_vec3;
	glm::vec2 temp_vec2;
	GLint temp_glint = 0;

	if (!in_file.is_open())
	{
		throw "Error: Could not open OBJ file";
	}

	while (std::getline(in_file, line))
	{
		ss.clear();
		ss.str(line);
		ss >> prefix;
		// For each line check prefix
		if (prefix == "#")
		{
			// This could be expanded to support .mtl's. It would be a good alternative to the strict folder structure /naming required at current to load textures
		}

		else if (prefix == "o")
		{

		}

		else if (prefix == "s")
		{

		}

		else if (prefix == "g")
		{

		}

		else if (prefix == "use_mtl")
		{

		}

		else if (prefix == "v") // vertex position
		{
			ss >> temp_vec3.x >> temp_vec3.y >> temp_vec3.z;
			vertex_positions.push_back(temp_vec3);
		}
		else if (prefix == "vt") // vertex texcoords
		{
			ss >> temp_vec2.x >> temp_vec2.y;
			vertex_texcoords.push_back(temp_vec2);
		}
		else if (prefix == "vn") // vertex normals
		{
			ss >> temp_vec3.x >> temp_vec3.y >> temp_vec3.z;
			vertex_normals.push_back(temp_vec3);
		}
		
		else if (prefix == "f")
		{
			int counter = 0;
			while (ss >> temp_glint)
			{
				// Pushing vert positions, texcoords and normals into appropriate arrays
				if (counter == 0)
				{
					vertex_position_indices.push_back(temp_glint);
				}
				else if (counter == 1)
				{
					vertex_texcoord_indices.push_back(temp_glint);
				}
				else if (counter == 2)
				{
					vertex_normal_indices.push_back(temp_glint);
				}
				// Handling extra characters
				if (ss.peek() == '/')
				{
					++counter;
					ss.ignore(1, '/');
				}
				else if (ss.peek() == ' ')
				{
					++counter;
					ss.ignore(1, ' ');
				}

				if (counter > 2)
				{
					counter = 0;
				}
			}
		}

		else
		{

		}
		
	}

	vertices.resize(vertex_position_indices.size(), Vertex());

	for (size_t i = 0; i < vertices.size(); ++i)
	{
		vertices[i].position = vertex_positions[vertex_position_indices[i] - 1];
		vertices[i].texcoord = vertex_texcoords[vertex_texcoord_indices[i] - 1];
		vertices[i].normal = vertex_normals[vertex_normal_indices[i] - 1];
		vertices[i].color = glm::vec3(1.0f, 1.0f, 1.0f);
	}
	int outCount = 0;
	// Calculate Tangent for TBN
	for (size_t i = 0; i <= vertices.size(); i += 3)
	{
		// Get triangle vertices
		Vertex& v0 = vertices[i];
		Vertex& v1 = vertices[i+1];
		Vertex& v2 = vertices[i+2];
		// Calculate triangle edges
		glm::vec3 edge1 = v1.position - v0.position;
		glm::vec3 edge2 = v2.position - v0.position;
		// calculate delta UV coordinates
		float deltaU1 = v1.texcoord.x - v0.texcoord.x;
		float deltaV1 = v1.texcoord.y - v0.texcoord.y;
		float deltaU2 = v2.texcoord.x - v0.texcoord.x;
		float deltaV2 = v2.texcoord.y - v0.texcoord.y;
		// Calculate fractional part of Normal equation
		float f = 1.0f / (deltaU1 * deltaV2 - deltaU2 * deltaV1);

		// Calculate tangent
		glm::vec3 tangent;
		tangent.x = f * (deltaV2 * edge1.x - deltaV1 * edge2.x);
		tangent.y = f * (deltaV2 * edge1.y - deltaV1 * edge2.y);
		tangent.z = f * (deltaV2 * edge1.z - deltaV1 * edge2.z);

		// Calculate bitangent
		glm::vec3 bitangent;
		bitangent.x = f * (-deltaU2 * edge1.x + deltaU1 * edge2.x);
		bitangent.y = f * (-deltaU2 * edge1.y + deltaU1 * edge2.y);
		bitangent.z = f * (-deltaU2 * edge1.z + deltaU1 * edge2.z);

		//Normalize
		tangent = glm::normalize(tangent);
		bitangent = glm::normalize(bitangent);
		// Update vertex with calculated tangent / bitangent
		v0.tangent += tangent;
		v1.tangent += tangent;
		v2.tangent += tangent;
		v0.bitangent += bitangent;
		v1.bitangent += bitangent;
		v2.bitangent += bitangent;
	}
	return vertices;
}