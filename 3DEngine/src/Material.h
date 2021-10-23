#pragma once

// GLEW
#include <glew.h>

// GLFW
#include <glfw3.h>

// MTB
#include <glm.hpp>
#include <vec2.hpp>
#include <vec3.hpp>
#include <vec4.hpp>
#include <mat4x4.hpp>
#include <gtc\type_ptr.hpp>

// Other
#include "Shader.h"

class Material
{
private:
	bool PBR; // Tracks if material is PBR or not
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	GLint diffuseTex;
	GLint specularTex;
	GLint albedoTex;
	GLint metalTex;
	GLint roughTex;
	GLint normTex;

public:
	// Blinn Phong constructor
	Material(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, GLint diffuseTex, GLint specularTex)
	{
		this->PBR = false;
		this->ambient = ambient;
		this->diffuse = diffuse;
		this->specular = specular;
		this->diffuseTex = diffuseTex;
		this->specularTex = specularTex;
	}
	// PBR constructor
	Material(glm::vec3 ambient, GLint albedoTex, GLint metalTex, GLint roughTex, GLint normTex)
	{
		this->PBR = true;
		this->ambient = ambient;
		this->albedoTex = albedoTex;
		this->metalTex = metalTex;
		this->roughTex = roughTex;
		this->normTex = normTex;
	}

	~Material()
	{

	}

	// Update material Uniforms
	void sendToShader(Shader &program)
	{
		if (!PBR)
		{
			program.setVec3f(this->ambient, "material.ambient");
			program.setVec3f(this->diffuse, "material.diffuse");
			program.setVec3f(this->specular, "material.specular");
			program.set1i(this->diffuseTex, "material.diffuseTex");
			program.set1i(this->specularTex, "material.specularTex");
		}
		else
		{
			program.setVec3f(this->ambient, "material.ambient");
			program.set1i(this->albedoTex, "material.albedoTex");
			program.set1i(this->metalTex, "material.metalTex");
			program.set1i(this->roughTex, "material.roughTex");
			program.set1i(this->normTex, "material.normTex");
		}
	}

};