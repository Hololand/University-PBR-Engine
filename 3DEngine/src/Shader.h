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


// OTHER
#include <iostream>
#include <fstream>
#include <string>

class Shader
{
private:

	GLuint id;

	// Read shader source
	std::string loadShaderSource(char* fileName)
	{
		std::string temp = "";
		std::string src = "";
		std::ifstream inFile;

		inFile.open(fileName);
		if (inFile.is_open())
		{
			while (std::getline(inFile, temp))
				src += temp + "\n";
		}
		else {
		std::cout << "ERROR: Could not open shader: " << fileName << std::endl;
		}
		inFile.close();
		return src;
	}

	// Load and compile shader
	GLuint loadShader(GLenum type, char* fileName)
	{
		char infoLog[512];
		GLint success;

		GLuint shader = glCreateShader(type);
		std::string str_src = this->loadShaderSource(fileName);
		const GLchar* src = str_src.c_str();
		glShaderSource(shader, 1, &src, NULL);
		glCompileShader(shader);

		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 512, NULL, infoLog);
			std::cout << "ERROR: Could not compile shader: " << fileName << std::endl;
			std::cout << infoLog << std::endl;
		}

		return shader;
	}

	// Link shader
	void linkProgram(GLuint vertexShader, GLuint geometryShader, GLuint fragmentShader)
	{
		char infoLog[512];
		GLint success;

		this->id = glCreateProgram();

		glAttachShader(this->id, vertexShader);

		if (geometryShader)
		{
			glAttachShader(this->id, geometryShader);
		}

		glAttachShader(this->id, fragmentShader);

		glLinkProgram(this->id);

		glGetProgramiv(this->id, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(this->id, 512, NULL, infoLog);
			std::cout << "ERROR: could not link program" << std::endl;
			std::cout << infoLog << std::endl;
		}
		glUseProgram(0);
	}
public:

	Shader(const char* vertexFile, const char* fragmentFile, const char* geometryFile = "")
	{
		GLuint vertexShader = 0;
		GLuint geometryShader = 0;
		GLuint fragmentShader = 0;

		//Load and Compile
		vertexShader = loadShader(GL_VERTEX_SHADER, (char*)vertexFile);
		if (geometryFile != "")
		{
			geometryShader = loadShader(GL_GEOMETRY_SHADER, (char*)geometryFile);
		}
		fragmentShader = loadShader(GL_FRAGMENT_SHADER, (char*)fragmentFile);

		//Link
		this->linkProgram(vertexShader, geometryShader, fragmentShader);

		//End
		glDeleteShader(vertexShader);
		glDeleteShader(geometryShader);
		glDeleteShader(fragmentShader);
	}
	~Shader()
	{
		glDeleteProgram(this->id);
	}

	//Set uniform functions
	void use()
	{
		glUseProgram(this->id);
	}

	void unuse()
	{
		glUseProgram(0);
	}

	void set1i(GLint value, const GLchar* name)
	{
		this->use();

		glUniform1i(glGetUniformLocation(this->id, name), value);

		this->unuse();
	}
	// Using unsigned int rather than GLint (Fix for data loss when parsing between the two)
	void set1iUI(unsigned int value, const GLchar* name)
	{
		this->use();

		glUniform1i(glGetUniformLocation(this->id, name), value);

		this->unuse();
	}

	void set1f(GLfloat value, const GLchar* name)
	{
		this->use();

		glUniform1f(glGetUniformLocation(this->id, name), value);

		this->unuse();
	}

	void setVec2f(glm::fvec2 value, const GLchar* name)
	{
		this->use();

		glUniform2fv(glGetUniformLocation(this->id, name), 1, glm::value_ptr(value));

		this->unuse();
	}

	void setVec3f(glm::fvec3 value, const GLchar* name)
	{
		this->use();

		glUniform3fv(glGetUniformLocation(this->id, name), 1, glm::value_ptr(value));

		this->unuse();
	}

	void setVec4f(glm::fvec4 value, const GLchar* name)
	{
		this->use();

		glUniform4fv(glGetUniformLocation(this->id, name), 1, glm::value_ptr(value));

		this->unuse();
	}

	void setMat3fv(glm::mat3 value, GLchar* name, GLboolean transpose = GL_FALSE)
	{
		this->use();

		glUniformMatrix3fv(glGetUniformLocation(this->id, name), 1, transpose, glm::value_ptr(value));

		this->unuse();
	}

	void setMat4fv(glm::mat4 value, const GLchar* name, GLboolean transpose = GL_FALSE)
	{
		this->use();

		glUniformMatrix4fv(glGetUniformLocation(this->id, name), 1, transpose, glm::value_ptr(value));

		this->unuse();
	}

};