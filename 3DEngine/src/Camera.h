#pragma once
// GLEW
#include <glew.h>

// GLFW
#include <glfw3.h>

// MTB
#include <glm.hpp>
#include <vec3.hpp>
#include <mat4x4.hpp>
#include <gtc\matrix_transform.hpp>

// OTHER
#include <iostream>

enum direction {FORWARD = 0, BACKWARD, LEFT, RIGHT, UP, DOWN};

class Camera
{
private:

	glm::mat4 viewMatrix;
	// For keeping track of which way is up when rotating the camera
	glm::vec3 worldUp;
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 right;
	glm::vec3 up;

	GLfloat pitch;
	GLfloat yaw;
	GLfloat roll; // Not really needed but usefull to have just incase

	void updateCameraVectors()
	{
		this->front.x = cos(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
		this->front.y = sin(glm::radians(this->pitch));
		this->front.z = sin(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));

		this->front = glm::normalize(this->front);
		this->right = glm::normalize(glm::cross(this->front, this->worldUp));
		this->up= glm::normalize(glm::cross(this->right, this->front));
	}

public:
	// Camera movement speed multipler and camera pan sensitivity multiplier
	GLfloat movementSpeed;
	GLfloat sensitivity;

	Camera(glm::vec3 position, glm::vec3 direction, glm::vec3 worldUp)
	{
		// Setting default camera settings
		this->viewMatrix = glm::mat4(1.0f);
		this->movementSpeed = 3.0f;
		this->sensitivity = 25.0f;

		this->worldUp = worldUp;
		this->position = position;
		this->right = glm::vec3(0.0f);
		this->up = worldUp;

		this->pitch = 0.0f;
		this->yaw = -90.0f;
		this->roll = 0.0f;

		this->updateCameraVectors();
	}

	~Camera()
	{

	}

	const glm::mat4 getViewMatix()
	{
		this->updateCameraVectors();
		this->viewMatrix = glm::lookAt(this->position, this->position + this->front, this->up);
		return this->viewMatrix;
	}

	const glm::vec3 getPosition()
	{
		return this->position;
	}


	void updateMouseInput(const float& dt, const double& offsetX, const double& offsetY)
	{
		// Update camera rotation according to mouse input from GLFW
		this->pitch += static_cast<GLfloat>(offsetY) * this->sensitivity * dt;
		this->yaw += static_cast<GLfloat>(offsetX) * this->sensitivity * dt;
		// Prevent excessive pitch and yaw which can cause issues and undersirable effects.
		// For example excessive pitch at 90 degrees can flip the screen
		if (this->pitch >= 85.0f) {
			this->pitch = 85.0f;
		}
		else if (this->pitch <= -85.0f) {
			this->pitch = -85.0f;
		}
		if (this->yaw > 360.0f || this->yaw < -360.0f) {
			this->yaw = 0.0f;
		}
	}
	// Update camera position based on keyboard inputs from GLFW and movevment speed
	void move(const float& dt, const int direction)
	{
		switch (direction)
		{
		case FORWARD:
			this->position += this->front * this->movementSpeed * dt;
			break;
		case BACKWARD:
			this->position -= this->front * this->movementSpeed * dt;
			break;
		case LEFT:
			this->position -= this->right * this->movementSpeed * dt;
			break;
		case RIGHT:
			this->position += this->right * this->movementSpeed * dt;
			break;
		case UP:
			this->position += this->up * this->movementSpeed * dt;
			break;
		case DOWN:
			this->position -= this->up * this->movementSpeed * dt;
			break;
		default:
			break;
		}
	}

	void updateInput(const float& dt, const int direction, const double& offsetX, const double& offsetY)
	{
		this->updateMouseInput(dt, offsetX, offsetY);
	}

	void setMoveSpeed(GLfloat moveSpeed)
	{
		this->movementSpeed = moveSpeed;
	}

	void setSens(GLfloat sensitivity)
	{
		this->sensitivity = sensitivity;
	}

};