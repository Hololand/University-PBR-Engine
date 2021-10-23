#pragma once

#include"Mesh.h"
#include"Texture.h"
#include"Shader.h"
#include"Material.h"
#include"OBJParser.h"

class Model
{

private:
	Material* material;
	Texture* overrideTextureDiffuse;
	Texture* overrideTextureSpecular;
	Texture* overrideTextureAlbedo;
	Texture* overrideTextureMetal;
	Texture* overrideTextureRough;
	Texture* overrideTextureNormal;
	std::vector<Mesh*> meshes;
	glm::vec3 position;

	void updateUniforms()
	{

	}

public:
	// Deprecated constructor, Was usefull before the OBJ loader was implemented (With primitives etc..)
	Model(glm::vec3 position, Material* material, Texture* texDif, Texture* texSpec, std::vector<Mesh*> meshes)
	{
		this->position = position;
		this->material = material;
		this->overrideTextureDiffuse = texDif;
		this->overrideTextureSpecular = texSpec;

		for (auto* i : meshes)
		{
			this->meshes.push_back(new Mesh(*i));
		}

		for (auto& i : this->meshes)
		{
			i->move(this->position);
			i->setOrigin(this->position);
		}

	}
	// Create Blinn Phong model from OBJ file
	Model(glm::vec3 position, Material* material, Texture* texDif, Texture* texSpec, const char* objFile)
	{
		// Get position, material and texture overrides
		this->position = position;
		this->material = material;
		this->overrideTextureDiffuse = texDif;
		this->overrideTextureSpecular = texSpec;
		// Load all OBJ meshes
		std::vector<Vertex> mesh = loadOBJ(objFile);
		this->meshes.push_back(new Mesh(mesh.data(), mesh.size(), NULL, 0, glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(.05f)));
		// Set meshes relative to model origin
		for (auto& i : this->meshes)
		{
			i->move(this->position);
			i->setOrigin(this->position);
		}

	}
	// Create PBR model from OBJ file
	Model(glm::vec3 position, Material* material, Texture* texAlbedo, Texture* texMetal, Texture* texRough, Texture* texNormal, const char* objFile)
	{
		// Get position, material and texture overrides.
		this->position = position;
		this->material = material;
		this->overrideTextureAlbedo = texAlbedo;
		this->overrideTextureMetal = texMetal;
		this->overrideTextureRough = texRough;
		this->overrideTextureNormal = texNormal;
		// Load all OBJ meshes
		std::vector<Vertex> mesh = loadOBJ(objFile);
		this->meshes.push_back(new Mesh(mesh.data(), mesh.size(), NULL, 0, glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(.05f)));
		// set meshes relative to model origin
		for (auto& i : this->meshes)
		{
			i->move(this->position);
			i->setOrigin(this->position);
		}

	}

	~Model()
	{
		for (auto*& i : this->meshes)
		{
			delete i;
		}
	}
	// Transformation functions
	void rotate(const glm::vec3 rotation)
	{
		for (auto& i : this->meshes)
			i->setRotation(rotation);
	}

	void scale(const glm::vec3 scale)
	{
		for (auto& i : this->meshes)
			i->setScale(scale);
	}

	void translate(const glm::vec3 translation)
	{
		for (auto& i : this->meshes)
			i->setPosition(translation);
	}
	// Update uniforms
	void update()
	{
		this->updateUniforms();
	}

	void render(Shader* shader)
	{
		// Update uniforms
		this->updateUniforms();

		// Update material uniform
		this->material->sendToShader(*shader);
		shader->use();

		// Bind new textures
		for (auto& i : this->meshes)
		{
			this->overrideTextureDiffuse->bind(0);
			this->overrideTextureSpecular->bind(1);

			i->render(shader);
		}
		
	}

	void renderPBR(Shader* shader)
	{
		// Update uniforms
		this->updateUniforms();

		// Update material Uniform
		this->material->sendToShader(*shader);
		shader->use();

		// Bind new textures
		for (auto& i : this->meshes)
		{
			this->overrideTextureAlbedo->bind(0);
			this->overrideTextureMetal->bind(1);
			this->overrideTextureRough->bind(2);
			this->overrideTextureNormal->bind(3);
			i->render(shader);
		}

	}

};