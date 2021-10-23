#pragma once

#include"libs.h"
// Parent class allowing more types of light to be created in the future
class Light
{
public:
	Light(float intensity, glm::vec3 colour)
	{
		this->intensity = intensity;
		this->colour = colour;
	}
	~Light()
	{

	}

	virtual void sendToShader(Shader& program) = 0;

protected:

	float intensity;
	glm::vec3 colour;

};

class PointLight : Light
{
protected:
	glm::vec3 position;
	// Deprecated variables for calculating attenuation
	float constant;
	float flinear;
	float quadratic;
public:
	PointLight(glm::vec3 position, float intensity = 1.0f, glm::vec3 colour = glm::vec3(1.0f), float constant = 1.0f, float linear = 0.045f, float quadratic = 0.0075f)
		: Light(intensity, colour)
	{
		this->position = position;
		// Deprecated variables for calculating attentuation
		this->constant = constant;
		this->flinear = linear; 
		this->quadratic = quadratic;
	}
	~PointLight()
	{

	}
	// Setters
	void setPosition(const glm::vec3 position)
	{
		this->position = position;
	}

	void setIntensity(float intensity)
	{
		this->intensity = intensity;
	}

	void setColour(const glm::vec3 colour)
	{
		this->colour = colour;
	}
	// Update light uniforms
	void sendToShader(Shader& program)
	{
		program.setVec3f(this->position, "pointLight.position");
		program.set1f(this->intensity, "pointLight.intensity");
		program.setVec3f(this->colour, "pointLight.colour");
		program.set1f(this->constant, "pointLight.constant");
		program.set1f(this->flinear, "pointLight.flinear");
		program.set1f(this->quadratic, "pointLight.quadratic");
	}
};